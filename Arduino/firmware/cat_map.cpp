#include "cat_map.h"

#include <string.h>
#include <stdbool.h>

#ifdef ARDUINO
#include <avr/pgmspace.h>
#else
#define PROGMEM
#define pgm_read_byte(p) (*(const uint8_t *)(p))
#endif

/*
 * FT-847 (host) -> FT-736R (radio) translator.
 * Opcode behavior follows hamspot/hs736usb ProcessIO (N6BIL, GPL-3)
 * cross-checked with Hamlib rigs/yaesu/ft847.c and ft736.c.
 */

static uint8_t freq_main[CAT_BLOCK];
static uint8_t freq_sat_rx[CAT_BLOCK];
static uint8_t freq_sat_tx[CAT_BLOCK];
static uint8_t rx_status[CAT_BLOCK];
static uint8_t tx_status[CAT_BLOCK];
static bool s_cat_on;
static bool s_ptt;
static bool s_sat;
static volatile cat_proto_t s_proto;
static volatile cat_proto_t s_jumper;
static volatile cat_src_t s_src;

/* PIC Tone[38]: 847 tone index -> 736 FA code. */
static const uint8_t k_tone[] PROGMEM = {
    0x1E, 0x20, 0x22, 0x24, 0x26, 0x28, 0x2A, 0x2C, 0x2E, 0x30,
    0x32, 0x34, 0x36, 0x38, 0x16, 0x1A, 0x1F, 0x21, 0x23, 0x25,
    0x27, 0x29, 0x2B, 0x2D, 0x2F, 0x31, 0x33, 0x35, 0x37, 0x39,
    0x18, 0x1C, 0x13, 0x15, 0x17, 0x19, 0x1B, 0x1D
};

#define TONE_N ((uint8_t)(sizeof(k_tone) / sizeof(k_tone[0])))

static void emit_radio(cat_result_t *out, const uint8_t *blk)
{
    if (out->n_radio >= CAT_MAX_RADIO_FRAMES) {
        return;
    }
    memcpy(out->radio[out->n_radio], blk, CAT_BLOCK);
    out->n_radio++;
}

static void emit_host(cat_result_t *out, const uint8_t *blk)
{
    memcpy(out->host, blk, CAT_BLOCK);
    out->n_host = CAT_BLOCK;
}

void cat_map_debug_dump(uint8_t out[CAT_DEBUG_LEN])
{
    memset(out, 0, CAT_DEBUG_LEN);
    out[0] = 0xA5;
    if (s_cat_on) {
        out[1] = (uint8_t)(out[1] | 0x01u);
    }
    if (s_ptt) {
        out[1] = (uint8_t)(out[1] | 0x02u);
    }
    if (s_sat) {
        out[1] = (uint8_t)(out[1] | 0x04u);
    }
    out[2] = cat_band_mask();
    out[3] = (uint8_t)((uint8_t)s_proto | ((uint8_t)s_src << 4));
    memcpy(out + 4, freq_main, CAT_BLOCK);
    memcpy(out + 9, freq_sat_rx, CAT_BLOCK);
    memcpy(out + 14, freq_sat_tx, CAT_BLOCK);
    memcpy(out + 19, rx_status, CAT_BLOCK);
    memcpy(out + 24, tx_status, CAT_BLOCK);
}

/* 23 cm: host shows 240-300 MHz; radio wants +1 GHz on the first BCD byte. */
static void apply_23cm(uint8_t *blk)
{
    if (blk[0] > 0x23 && blk[0] < 0x31) {
        blk[0] = (uint8_t)(blk[0] + 0xA0);
    }
}

static void cache_freq4(uint8_t *cache, const uint8_t *cmd)
{
    cache[0] = cmd[0];
    cache[1] = cmd[1];
    cache[2] = cmd[2];
    cache[3] = cmd[3];
}

static void fill_status(uint8_t *buf, uint8_t fill, uint8_t opcode)
{
    buf[0] = fill;
    buf[1] = fill;
    buf[2] = fill;
    buf[3] = fill;
    buf[4] = opcode;
}

void cat_map_init(void)
{
    /* 145.0000 MHz FM — same defaults as the PIC. */
    freq_main[0] = 0x14;
    freq_main[1] = 0x50;
    freq_main[2] = 0x00;
    freq_main[3] = 0x00;
    freq_main[4] = 0x08;

    freq_sat_rx[0] = 0x14;
    freq_sat_rx[1] = 0x50;
    freq_sat_rx[2] = 0x00;
    freq_sat_rx[3] = 0x00;
    freq_sat_rx[4] = 0x00;

    freq_sat_tx[0] = 0x44;
    freq_sat_tx[1] = 0x00;
    freq_sat_tx[2] = 0x00;
    freq_sat_tx[3] = 0x00;
    freq_sat_tx[4] = 0x00;

    fill_status(rx_status, 0x80, 0xE7);
    fill_status(tx_status, 0x80, 0xF7);
    s_cat_on = false;
    s_ptt = false;
    s_sat = false;
    s_proto = CAT_PROTO_847;
    s_jumper = CAT_PROTO_847;
    s_src = CAT_SRC_JUMPER;
}

static cat_proto_t effective_proto(void)
{
    if (s_src == CAT_SRC_847) {
        return CAT_PROTO_847;
    }
    if (s_src == CAT_SRC_736) {
        return CAT_PROTO_736;
    }
    return s_jumper;
}

static bool apply_effective(void)
{
    cat_proto_t next;

    next = effective_proto();
    if (next == s_proto) {
        return false;
    }
    s_proto = next;
    return true;
}

void cat_map_set_proto(cat_proto_t proto)
{
    if (proto == CAT_PROTO_736) {
        s_src = CAT_SRC_736;
    } else {
        s_src = CAT_SRC_847;
    }
    s_proto = proto;
}

cat_proto_t cat_map_proto(void)
{
    return s_proto;
}

void cat_map_set_src(cat_src_t src)
{
    s_src = src;
    (void)apply_effective();
}

cat_src_t cat_map_src(void)
{
    return s_src;
}

bool cat_map_note_jumper(cat_proto_t jumper)
{
    s_jumper = jumper;
    return apply_effective();
}

/* 736 1.2 GHz uses 0xC_ in the first BCD byte. 847 cache shows 240 MHz. */
static void cache_freq4_host(uint8_t *cache, const uint8_t *cmd)
{
    cache_freq4(cache, cmd);
    if ((cache[0] & 0xF0) == 0xC0) {
        cache[0] = (uint8_t)((cache[0] & 0x0F) | 0x20);
    }
}

uint8_t cat_band_from_bcd0(uint8_t b0)
{
    if (b0 == 0x05) {
        return CAT_BAND_50;
    }
    if (b0 == 0x14 || b0 == 0x15) {
        return CAT_BAND_144;
    }
    if (b0 == 0x22) {
        return CAT_BAND_220;
    }
    if (b0 == 0x43 || b0 == 0x44) {
        return CAT_BAND_430;
    }
    /* Host shows 240–300 MHz for 23 cm. */
    if (b0 > 0x23 && b0 < 0x31) {
        return CAT_BAND_1240;
    }
    return 0;
}

bool cat_is_on(void)
{
    return s_cat_on;
}

bool cat_ptt(void)
{
    return s_ptt;
}

bool cat_sat(void)
{
    return s_sat;
}

uint8_t cat_bcd0_main(void)
{
    return freq_main[0];
}

uint8_t cat_bcd0_sat_rx(void)
{
    return freq_sat_rx[0];
}

uint8_t cat_bcd0_sat_tx(void)
{
    return freq_sat_tx[0];
}

uint8_t cat_band_mask(void)
{
    uint8_t mask;

    if (s_sat) {
        mask = (uint8_t)(cat_band_from_bcd0(freq_sat_rx[0]) |
                         cat_band_from_bcd0(freq_sat_tx[0]));
    } else {
        mask = cat_band_from_bcd0(freq_main[0]);
    }
    return mask;
}

static void ptt_on(cat_result_t *out, uint8_t *w)
{
    s_ptt = true;
    emit_radio(out, w);
    fill_status(rx_status, 0x40, 0xE7);
    fill_status(tx_status, 0x00, 0xF7);
}

static void ptt_off(cat_result_t *out, uint8_t *w)
{
    s_ptt = false;
    emit_radio(out, w);
    fill_status(rx_status, 0x47, 0xE7);
    fill_status(tx_status, 0x80, 0xF7);
}

static void dispatch_847(uint8_t *w, uint8_t op, cat_result_t *out)
{
    /* Native 736 sat/tone opcodes are locked out in 847 mode. */
    if (op == 0xE7) {
        emit_host(out, rx_status);
        return;
    }
    if (op == 0xF7) {
        emit_host(out, tx_status);
        return;
    }
    if (op == 0x03) {
        emit_host(out, freq_main);
        return;
    }
    if (op == 0x13) {
        emit_host(out, freq_sat_rx);
        return;
    }
    if (op == 0x23) {
        emit_host(out, freq_sat_tx);
        return;
    }
    if (op == 0x00) {
        s_cat_on = true;
        emit_radio(out, w);
        memcpy(w, freq_main, 4);
        w[4] = 0x01;
        emit_radio(out, w);
        w[0] = 0x08;
        w[1] = 0x00;
        w[2] = 0x00;
        w[3] = 0x00;
        w[4] = 0x07;
        emit_radio(out, w);
        return;
    }
    if (op == 0x80) {
        s_cat_on = false;
        s_ptt = false;
        emit_radio(out, w);
        return;
    }
    if (op == 0x01) {
        cache_freq4(freq_main, w);
        apply_23cm(w);
        emit_radio(out, w);
        return;
    }
    if (op == 0x11) {
        cache_freq4(freq_sat_rx, w);
        w[4] = 0x1E;
        apply_23cm(w);
        emit_radio(out, w);
        return;
    }
    if (op == 0x21) {
        cache_freq4(freq_sat_tx, w);
        w[4] = 0x2E;
        apply_23cm(w);
        emit_radio(out, w);
        return;
    }
    if (op == 0x07) {
        freq_main[4] = w[0];
        emit_radio(out, w);
        return;
    }
    if (op == 0x17) {
        freq_sat_rx[4] = w[0];
        emit_radio(out, w);
        return;
    }
    if (op == 0x27) {
        freq_sat_tx[4] = w[0];
        emit_radio(out, w);
        return;
    }
    if (op == 0x08) {
        ptt_on(out, w);
        return;
    }
    if (op == 0x88) {
        ptt_off(out, w);
        return;
    }
    if (op == 0x4E) {
        s_sat = true;
        w[4] = 0x0E;
        emit_radio(out, w);
        return;
    }
    if (op == 0x8E) {
        s_sat = false;
        emit_radio(out, w);
        memcpy(w, freq_main, 4);
        w[4] = 0x01;
        emit_radio(out, w);
        return;
    }
    if (op == 0x09) {
        w[4] = w[0];
        emit_radio(out, w);
        return;
    }
    if (op == 0x49 || op == 0x89) {
        emit_radio(out, w);
        return;
    }
    if (op == 0x0A) {
        if (w[0] == 0x0A) {
            return;
        }
        w[4] = w[0];
        if (w[0] == 0x2A) {
            w[4] = 0x0A;
        }
        emit_radio(out, w);
        return;
    }
    if (op == 0x0B) {
        uint8_t idx;

        if (w[0] > 31) {
            idx = (uint8_t)(w[0] - 26);
        } else {
            idx = w[0];
        }
        if (idx >= TONE_N) {
            return;
        }
        w[0] = pgm_read_byte(&k_tone[idx]);
        w[4] = 0xFA;
        emit_radio(out, w);
        return;
    }
    if (op == 0xF9) {
        emit_radio(out, w);
        return;
    }
}

static void dispatch_736(uint8_t *w, uint8_t op, cat_result_t *out)
{
    /* 847-only opcodes (4E, 11, 21, 0B) are locked out. */
    if (op == 0xE7 || op == 0xF7) {
        emit_radio(out, w);
        out->radio_reply_len = 5;
        return;
    }
    if (op == 0x03) {
        emit_host(out, freq_main);
        return;
    }
    if (op == 0x13) {
        emit_host(out, freq_sat_rx);
        return;
    }
    if (op == 0x23) {
        emit_host(out, freq_sat_tx);
        return;
    }
    if (op == 0x00) {
        s_cat_on = true;
        emit_radio(out, w);
        return;
    }
    if (op == 0x80) {
        s_cat_on = false;
        s_ptt = false;
        emit_radio(out, w);
        return;
    }
    if (op == 0x01) {
        cache_freq4_host(freq_main, w);
        emit_radio(out, w);
        return;
    }
    if (op == 0x1E) {
        cache_freq4_host(freq_sat_rx, w);
        s_sat = true;
        emit_radio(out, w);
        return;
    }
    if (op == 0x2E) {
        cache_freq4_host(freq_sat_tx, w);
        s_sat = true;
        emit_radio(out, w);
        return;
    }
    if (op == 0x07) {
        freq_main[4] = w[0];
        emit_radio(out, w);
        return;
    }
    if (op == 0x17) {
        freq_sat_rx[4] = w[0];
        emit_radio(out, w);
        return;
    }
    if (op == 0x27) {
        freq_sat_tx[4] = w[0];
        emit_radio(out, w);
        return;
    }
    if (op == 0x08) {
        ptt_on(out, w);
        return;
    }
    if (op == 0x88) {
        ptt_off(out, w);
        return;
    }
    if (op == 0x0E) {
        s_sat = true;
        emit_radio(out, w);
        return;
    }
    if (op == 0x8E) {
        s_sat = false;
        emit_radio(out, w);
        return;
    }
    if (op == 0x09 || op == 0x49 || op == 0x89 || op == 0xF9) {
        emit_radio(out, w);
        return;
    }
    if (op == 0x0A || op == 0x4A || op == 0x8A || op == 0xFA) {
        emit_radio(out, w);
        return;
    }
}

void cat_map_dispatch(const uint8_t cmd[CAT_BLOCK], cat_result_t *out)
{
    uint8_t w[CAT_BLOCK];
    uint8_t op;
    cat_proto_t proto;

    memset(out, 0, sizeof(*out));
    memcpy(w, cmd, CAT_BLOCK);
    op = w[4];
    proto = s_proto;

    if (op == CAT_DEBUG_OP && w[0] == CAT_DEBUG_P0 && w[1] == CAT_DEBUG_P1 &&
        w[2] == CAT_DEBUG_P2 && w[3] == CAT_DEBUG_P3) {
        cat_map_debug_dump(out->host);
        out->n_host = CAT_DEBUG_LEN;
        return;
    }

    if (op == CAT_SRC_OP && w[0] == CAT_DEBUG_P0 && w[1] == CAT_DEBUG_P1 &&
        w[2] == CAT_DEBUG_P2) {
        if (w[3] == CAT_SRC_SEL_CYCLE) {
            s_src = (cat_src_t)(((uint8_t)s_src + 1u) % 3u);
        } else if (w[3] == CAT_SRC_SEL_JUMPER || w[3] == CAT_SRC_SEL_847 ||
                   w[3] == CAT_SRC_SEL_736) {
            s_src = (cat_src_t)w[3];
        }
        (void)apply_effective();
        cat_map_debug_dump(out->host);
        out->n_host = CAT_DEBUG_LEN;
        return;
    }

    if (proto == CAT_PROTO_736) {
        dispatch_736(w, op, out);
    } else {
        dispatch_847(w, op, out);
    }
}
