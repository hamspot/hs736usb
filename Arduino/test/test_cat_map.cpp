#include "../firmware/cat_map.h"
#include "../firmware/cat_frame.h"
#include "../firmware/gpio_logic.h"
#include "../firmware/encoder_logic.h"
#include "../firmware/proto_debounce.h"

#include <cstdio>
#include <cstdint>
#include <cstring>
#include <cstdlib>

static int g_fail;

static void expect_eq(const char *name, const uint8_t *got, const uint8_t *want, int n)
{
    if (memcmp(got, want, (size_t)n) != 0) {
        std::fprintf(stderr, "FAIL %s\n  got :", name);
        for (int i = 0; i < n; i++) {
            std::fprintf(stderr, " %02X", got[i]);
        }
        std::fprintf(stderr, "\n  want:");
        for (int i = 0; i < n; i++) {
            std::fprintf(stderr, " %02X", want[i]);
        }
        std::fprintf(stderr, "\n");
        g_fail++;
    }
}

static void expect_n(const char *name, int got, int want)
{
    if (got != want) {
        std::fprintf(stderr, "FAIL %s: n=%d want=%d\n", name, got, want);
        g_fail++;
    }
}

int main(void)
{
    cat_result_t r;
    uint8_t cmd[5];
    uint8_t frame[5];

    cat_map_init();
    cat_frame_init();

    /* Default 03 = 145.0000 FM */
    cmd[0] = 0; cmd[1] = 0; cmd[2] = 0; cmd[3] = 0; cmd[4] = 0x03;
    cat_map_dispatch(cmd, &r);
    expect_n("03 n_radio", r.n_radio, 0);
    expect_n("03 n_host", r.n_host, 5);
    {
        const uint8_t want[5] = {0x14, 0x50, 0x00, 0x00, 0x08};
        expect_eq("03 cache", r.host, want, 5);
    }

    /* CAT ON -> 00, cached freq 01, FM 07 */
    cmd[4] = 0x00;
    cat_map_dispatch(cmd, &r);
    expect_n("CAT ON n_radio", r.n_radio, 3);
    {
        const uint8_t a[5] = {0x00, 0x00, 0x00, 0x00, 0x00};
        const uint8_t b[5] = {0x14, 0x50, 0x00, 0x00, 0x01};
        const uint8_t c[5] = {0x08, 0x00, 0x00, 0x00, 0x07};
        expect_eq("CAT ON 00", r.radio[0], a, 5);
        expect_eq("CAT ON 01", r.radio[1], b, 5);
        expect_eq("CAT ON 07", r.radio[2], c, 5);
    }

    /* 11 -> 1E, 21 -> 2E, 4E -> 0E */
    cmd[0] = 0x14; cmd[1] = 0x50; cmd[2] = 0x00; cmd[3] = 0x00; cmd[4] = 0x11;
    cat_map_dispatch(cmd, &r);
    expect_n("11 n", r.n_radio, 1);
    {
        const uint8_t want[5] = {0x14, 0x50, 0x00, 0x00, 0x1E};
        expect_eq("11->1E", r.radio[0], want, 5);
    }
    cmd[0] = 0x43; cmd[1] = 0x50; cmd[4] = 0x21;
    cat_map_dispatch(cmd, &r);
    {
        const uint8_t want[5] = {0x43, 0x50, 0x00, 0x00, 0x2E};
        expect_eq("21->2E", r.radio[0], want, 5);
    }
    cmd[4] = 0x4E;
    cat_map_dispatch(cmd, &r);
    expect_n("4E n", r.n_radio, 1);
    if (r.radio[0][4] != 0x0E) {
        std::fprintf(stderr, "FAIL 4E->0E opcode %02X\n", r.radio[0][4]);
        g_fail++;
    }

    /* 23 cm: host 240 MHz, radio C4, cache stays 24 */
    cmd[0] = 0x24; cmd[1] = 0x00; cmd[2] = 0x00; cmd[3] = 0x00; cmd[4] = 0x01;
    cat_map_dispatch(cmd, &r);
    {
        const uint8_t want[5] = {0xC4, 0x00, 0x00, 0x00, 0x01};
        expect_eq("23cm radio", r.radio[0], want, 5);
    }
    cmd[4] = 0x03;
    cat_map_dispatch(cmd, &r);
    {
        const uint8_t want[5] = {0x24, 0x00, 0x00, 0x00, 0x08};
        expect_eq("23cm cache", r.host, want, 5);
    }

    /* PTT on/off status byte0 */
    cmd[0] = 0; cmd[1] = 0; cmd[2] = 0; cmd[3] = 0; cmd[4] = 0x08;
    cat_map_dispatch(cmd, &r);
    cmd[4] = 0xF7;
    cat_map_dispatch(cmd, &r);
    if (r.host[0] != 0x00) {
        std::fprintf(stderr, "FAIL PTT ON tx status %02X\n", r.host[0]);
        g_fail++;
    }
    cmd[4] = 0x88;
    cat_map_dispatch(cmd, &r);
    cmd[4] = 0xF7;
    cat_map_dispatch(cmd, &r);
    if (r.host[0] != 0x80) {
        std::fprintf(stderr, "FAIL PTT OFF tx status %02X\n", r.host[0]);
        g_fail++;
    }

    /* DCS must not become CAT ON */
    cmd[0] = 0x0A; cmd[4] = 0x0A;
    cat_map_dispatch(cmd, &r);
    expect_n("DCS drop radio", r.n_radio, 0);
    expect_n("DCS drop host", r.n_host, 0);

    /* CTCSS index 0 and 32 (PIC >31 skip) */
    cmd[0] = 0x00; cmd[4] = 0x0B;
    cat_map_dispatch(cmd, &r);
    expect_n("tone0 n", r.n_radio, 1);
    if (r.radio[0][0] != 0x1E || r.radio[0][4] != 0xFA) {
        std::fprintf(stderr, "FAIL tone0 %02X op %02X\n", r.radio[0][0], r.radio[0][4]);
        g_fail++;
    }
    /* PIC: if cmd[0] > 31, index = cmd[0] - 26. 32 -> 6 -> Tone[6]=0x2A */
    cmd[0] = 32; cmd[4] = 0x0B;
    cat_map_dispatch(cmd, &r);
    if (r.n_radio != 1 || r.radio[0][0] != 0x2A) {
        std::fprintf(stderr, "FAIL tone32 n=%u b0=%02X\n", r.n_radio,
                     r.n_radio ? r.radio[0][0] : 0);
        g_fail++;
    }
    cmd[0] = 0x40; cmd[4] = 0x0B;
    cat_map_dispatch(cmd, &r);
    expect_n("tone OOB drop", r.n_radio, 0);

    /* Hamlib plus shift opcode 49 forwarded */
    cmd[0] = 0; cmd[4] = 0x49;
    cat_map_dispatch(cmd, &r);
    expect_n("49 n", r.n_radio, 1);
    if (r.radio[0][4] != 0x49) {
        std::fprintf(stderr, "FAIL 49 opcode %02X\n", r.radio[0][4]);
        g_fail++;
    }

    /* Frame assembler: two commands, gap discard */
    cat_frame_init();
    {
        const uint8_t pkt[] = {0x00, 0x00, 0x00, 0x00, 0x03, 0x00, 0x00, 0x00, 0x00, 0xE7};
        int n_done = 0;
        for (unsigned i = 0; i < sizeof(pkt); i++) {
            if (cat_frame_feed(pkt[i], 10 + i)) {
                cat_frame_take(frame);
                n_done++;
                if (n_done == 1) {
                    expect_eq("frame1", frame, pkt, 5);
                }
                if (n_done == 2) {
                    expect_eq("frame2", frame, pkt + 5, 5);
                }
            }
        }
        expect_n("two frames", n_done, 2);
    }
    cat_frame_init();
    (void)cat_frame_feed(0x01, 0);
    (void)cat_frame_feed(0x02, 10);
    if (cat_frame_feed(0x03, 300)) {
        /* 300-0 > 250: should have reset, so not complete */
        std::fprintf(stderr, "FAIL gap should reset\n");
        g_fail++;
    }

    cat_map_init();
    expect_n("band default 144", cat_band_from_bcd0(0x14), (int)CAT_BAND_144);
    expect_n("band 50", cat_band_from_bcd0(0x05), (int)CAT_BAND_50);
    expect_n("band 430", cat_band_from_bcd0(0x43), (int)CAT_BAND_430);
    expect_n("band 23cm", cat_band_from_bcd0(0x24), (int)CAT_BAND_1240);
    expect_n("mask init", cat_band_mask(), (int)CAT_BAND_144);
    cmd[0] = 0; cmd[1] = 0; cmd[2] = 0; cmd[3] = 0; cmd[4] = 0x00;
    cat_map_dispatch(cmd, &r);
    if (!cat_is_on()) {
        std::fprintf(stderr, "FAIL CAT on flag\n");
        g_fail++;
    }
    cmd[4] = 0x08;
    cat_map_dispatch(cmd, &r);
    if (!cat_ptt()) {
        std::fprintf(stderr, "FAIL CAT PTT\n");
        g_fail++;
    }
    cmd[4] = 0x4E;
    cat_map_dispatch(cmd, &r);
    if (!cat_sat()) {
        std::fprintf(stderr, "FAIL sat flag\n");
        g_fail++;
    }
    expect_n("pwm floor", gpio_smeter_pwm(0x30), 0);
    expect_n("pwm ceil", gpio_smeter_pwm(0xAD), 255);
    expect_n("poll idle", (int)gpio_meter_period_ms(10000, 0, 0), (int)GPIO_POLL_IDLE_MS);
    expect_n("poll after TX", (int)gpio_meter_period_ms(1000, 900, 0), (int)GPIO_POLL_ACTIVE_MS);
    expect_n("poll strong", (int)gpio_meter_period_ms(10000, 0, 84), (int)GPIO_POLL_ACTIVE_MS);
    expect_n("active is 3x cmd", (int)GPIO_POLL_ACTIVE_MS, (int)(3u * CAT_SMETER_CMD_MS));
    {
        uint16_t mid = gpio_smeter_slew_q8(0, 255, (uint16_t)(GPIO_POLL_ACTIVE_MS / 2),
                                           (uint16_t)GPIO_POLL_ACTIVE_MS);
        if ((mid >> 8) < 100 || (mid >> 8) > 155) {
            std::fprintf(stderr, "FAIL slew midpoint %u\n", mid >> 8);
            g_fail++;
        }
        expect_n("slew done", gpio_smeter_slew_q8(0, 200, 999, 100) >> 8, 200);
        expect_n("slew hold", gpio_smeter_slew_q8(10 << 8, 10, 5, 100) >> 8, 10);
    }
    expect_n("dots floor", gpio_smeter_dots(0x30), 0);
    expect_n("dots ceil", gpio_smeter_dots(0xAD), 31);
    expect_n("rx sql closed", gpio_smeter_rx_status(0x30, true), 0x80);
    expect_n("rx sql open", gpio_smeter_rx_status(0xAD, false), 0x1F);
    cat_map_note_smeter(0xAD, false);
    cmd[0] = 0; cmd[1] = 0; cmd[2] = 0; cmd[3] = 0; cmd[4] = 0xE7;
    cat_map_dispatch(cmd, &r);
    expect_n("E7 n_host", r.n_host, 5);
    expect_n("E7 meter", r.host[0], 0x1F);
    expect_n("E7 op", r.host[4], 0xE7);

    cmd[0] = CAT_DEBUG_P0;
    cmd[1] = CAT_DEBUG_P1;
    cmd[2] = CAT_DEBUG_P2;
    cmd[3] = CAT_DEBUG_P3;
    cmd[4] = CAT_DEBUG_OP;
    cat_map_dispatch(cmd, &r);
    expect_n("debug n_host", r.n_host, CAT_DEBUG_LEN);
    expect_n("debug n_radio", r.n_radio, 0);
    if (r.host[0] != 0xA5 || (r.host[1] & 0x01) == 0 || (r.host[1] & 0x04) == 0) {
        std::fprintf(stderr, "FAIL debug dump flags %02X %02X\n", r.host[0], r.host[1]);
        g_fail++;
    }

    /* 847 mode: native 736 sat opcodes locked out */
    cat_map_set_proto(CAT_PROTO_847);
    cmd[0] = 0; cmd[1] = 0; cmd[2] = 0; cmd[3] = 0; cmd[4] = 0x0E;
    cat_map_dispatch(cmd, &r);
    expect_n("847 locks 0E", r.n_radio, 0);
    cmd[4] = 0x1E;
    cat_map_dispatch(cmd, &r);
    expect_n("847 locks 1E", r.n_radio, 0);

    /* 736 mode: passthrough sat, lock 847 4E/11 */
    cat_map_set_proto(CAT_PROTO_736);
    cmd[4] = 0x0E;
    cat_map_dispatch(cmd, &r);
    expect_n("736 0E n", r.n_radio, 1);
    if (!cat_sat() || r.radio[0][4] != 0x0E) {
        std::fprintf(stderr, "FAIL 736 0E sat/op\n");
        g_fail++;
    }
    cmd[0] = 0x43; cmd[1] = 0x20; cmd[2] = 0; cmd[3] = 0; cmd[4] = 0x1E;
    cat_map_dispatch(cmd, &r);
    expect_n("736 1E n", r.n_radio, 1);
    if (r.radio[0][4] != 0x1E) {
        std::fprintf(stderr, "FAIL 736 1E opcode\n");
        g_fail++;
    }
    cmd[0] = 0; cmd[4] = 0x4E;
    cat_map_dispatch(cmd, &r);
    expect_n("736 locks 4E", r.n_radio, 0);
    cmd[4] = 0x00;
    cat_map_dispatch(cmd, &r);
    expect_n("736 CAT ON frames", r.n_radio, 1);
    cmd[4] = 0xE7;
    cat_map_dispatch(cmd, &r);
    expect_n("736 E7 radio", r.n_radio, 1);
    expect_n("736 E7 reply wait", r.radio_reply_len, 5);
    expect_n("736 E7 no fake host", r.n_host, 0);
    cat_map_set_proto(CAT_PROTO_847);

    /* A5 73 36 sel FC — three-way source (jumper / force 847 / force 736) */
    cat_map_init();
    cmd[0] = CAT_DEBUG_P0;
    cmd[1] = CAT_DEBUG_P1;
    cmd[2] = CAT_DEBUG_P2;
    cmd[3] = CAT_SRC_SEL_736;
    cmd[4] = CAT_SRC_OP;
    cat_map_dispatch(cmd, &r);
    expect_n("src 736 n_host", r.n_host, CAT_DEBUG_LEN);
    expect_n("src 736", (int)cat_map_src(), (int)CAT_SRC_736);
    expect_n("src 736 proto", (int)cat_map_proto(), (int)CAT_PROTO_736);
    cmd[0] = 0; cmd[1] = 0; cmd[2] = 0; cmd[3] = 0; cmd[4] = 0x0E;
    cat_map_dispatch(cmd, &r);
    expect_n("force 736 allows 0E", r.n_radio, 1);
    cmd[0] = CAT_DEBUG_P0;
    cmd[1] = CAT_DEBUG_P1;
    cmd[2] = CAT_DEBUG_P2;
    cmd[3] = CAT_SRC_SEL_847;
    cmd[4] = CAT_SRC_OP;
    cat_map_dispatch(cmd, &r);
    cmd[0] = 0; cmd[1] = 0; cmd[2] = 0; cmd[3] = 0; cmd[4] = 0x0E;
    cat_map_dispatch(cmd, &r);
    expect_n("force 847 locks 0E", r.n_radio, 0);
    (void)cat_map_note_jumper(CAT_PROTO_736);
    expect_n("force 847 ignores jumper", (int)cat_map_proto(), (int)CAT_PROTO_847);
    cmd[0] = CAT_DEBUG_P0;
    cmd[1] = CAT_DEBUG_P1;
    cmd[2] = CAT_DEBUG_P2;
    cmd[3] = CAT_SRC_SEL_JUMPER;
    cmd[4] = CAT_SRC_OP;
    cat_map_dispatch(cmd, &r);
    expect_n("jumper follows pin", (int)cat_map_proto(), (int)CAT_PROTO_736);
    cmd[3] = CAT_SRC_SEL_CYCLE;
    cat_map_dispatch(cmd, &r);
    expect_n("cycle to 847", (int)cat_map_src(), (int)CAT_SRC_847);
    cat_map_dispatch(cmd, &r);
    expect_n("cycle to 736", (int)cat_map_src(), (int)CAT_SRC_736);
    cat_map_dispatch(cmd, &r);
    expect_n("cycle to jumper", (int)cat_map_src(), (int)CAT_SRC_JUMPER);
    cat_map_init();

    {
        proto_debounce_t d;
        bool revert;
        proto_debounce_init(&d, 1);
        proto_debounce_irq(&d, 100);
        if (proto_debounce_poll(&d, 120, 0, &revert)) {
            std::fprintf(stderr, "FAIL bounce should wait 50ms\n");
            g_fail++;
        }
        proto_debounce_irq(&d, 125);
        if (proto_debounce_poll(&d, 160, 0, &revert)) {
            std::fprintf(stderr, "FAIL bounce restart\n");
            g_fail++;
        }
        if (!proto_debounce_poll(&d, 180, 0, &revert) || revert) {
            std::fprintf(stderr, "FAIL first stable edge\n");
            g_fail++;
        }
        proto_debounce_irq(&d, 400);
        if (!proto_debounce_poll(&d, 460, 1, &revert) || !revert) {
            std::fprintf(stderr, "FAIL double-flip revert\n");
            g_fail++;
        }
        proto_debounce_irq(&d, 2000);
        if (!proto_debounce_poll(&d, 2060, 0, &revert) || revert) {
            std::fprintf(stderr, "FAIL slow flip is not revert\n");
            g_fail++;
        }
    }

    expect_n("amfm AM", encoder_is_amfm(0x04), 1);
    expect_n("amfm USB", encoder_is_amfm(0x01), 0);
    expect_n("nibble AM", encoder_default_nibble(0x04), 5);
    expect_n("nibble USB", encoder_default_nibble(0x01), 6);
    expect_n("mode AM->FMN", encoder_next_mode(0x04), 0x88);
    expect_n("mode CW->AM", encoder_next_mode(0x02), 0x04);
    expect_n("cursor wrap", encoder_cursor_next(6), 0);
    {
        uint8_t bcd[4] = {0x14, 0x50, 0x00, 0x00};
        encoder_add_nibble(bcd, 5, 1);
        expect_n("1kHz up b0", bcd[0], 0x14);
        expect_n("1kHz up b1", bcd[1], 0x50);
        expect_n("1kHz up b2", bcd[2], 0x01);
        encoder_add_nibble(bcd, 5, -1);
        expect_n("1kHz down b2", bcd[2], 0x00);
    }
    {
        uint8_t prev = 0;
        int8_t s = 0;
        s += encoder_quad_step(&prev, 1);
        s += encoder_quad_step(&prev, 3);
        s += encoder_quad_step(&prev, 2);
        s += encoder_quad_step(&prev, 0);
        expect_n("quad cw", s, 4);
    }
    {
        encoder_btn_t st;
        encoder_btn_init(&st, 1);
        expect_n("btn idle", encoder_btn_poll(&st, 1, 10), ENC_BTN_NONE);
        expect_n("btn bounce", encoder_btn_poll(&st, 0, 20), ENC_BTN_NONE);
        expect_n("btn down", encoder_btn_poll(&st, 0, 60), ENC_BTN_NONE);
        expect_n("btn up wait", encoder_btn_poll(&st, 1, 100), ENC_BTN_NONE);
        expect_n("btn short", encoder_btn_poll(&st, 1, 140), ENC_BTN_SHORT);
        expect_n("btn down2", encoder_btn_poll(&st, 0, 180), ENC_BTN_NONE);
        expect_n("btn held", encoder_btn_poll(&st, 0, 220), ENC_BTN_NONE);
        expect_n("btn long", encoder_btn_poll(&st, 0, 900), ENC_BTN_LONG);
    }
    cat_map_init();
    cat_map_knob_freq(1, 5, &r);
    expect_n("knob n_radio", r.n_radio, 1);
    expect_n("knob op", r.radio[0][4], 0x01);
    expect_n("knob 1k", r.radio[0][2], 0x01);
    cat_map_knob_mode(&r);
    expect_n("knob mode op", r.radio[0][4], 0x07);
    expect_n("knob mode FM->LSB", r.radio[0][0], 0x00);
    expect_n("mode cache", cat_mode_main(), 0x00);

    if (g_fail) {
        std::fprintf(stderr, "%d failure(s)\n", g_fail);
        return 1;
    }
    std::puts("ok");
    return 0;
}
