#include "encoder_logic.h"

static const uint8_t k_modes[] = {
    0x04, 0x88, 0x08, 0x00, 0x01, 0x82, 0x02
};

#define K_MODES_N ((uint8_t)(sizeof(k_modes) / sizeof(k_modes[0])))

bool encoder_is_amfm(uint8_t mode)
{
    return mode == 0x04u || mode == 0x08u || mode == 0x88u;
}

uint8_t encoder_default_nibble(uint8_t mode)
{
    /* 1 kHz = nibble 5; 100 Hz = nibble 6. */
    return encoder_is_amfm(mode) ? 5u : 6u;
}

uint8_t encoder_next_mode(uint8_t mode)
{
    uint8_t i;

    for (i = 0; i < K_MODES_N; i++) {
        if (k_modes[i] == mode) {
            return k_modes[(uint8_t)((i + 1u) % K_MODES_N)];
        }
    }
    return k_modes[0];
}

uint8_t encoder_cursor_next(uint8_t cursor)
{
    if (cursor == ENC_CURSOR_DEFAULT || cursor >= ENC_NIBBLE_MAX) {
        return 0;
    }
    return (uint8_t)(cursor + 1u);
}

void encoder_add_nibble(uint8_t bcd[4], uint8_t nibble, int8_t dir)
{
    uint8_t i;
    uint8_t nib[8];
    int8_t d;

    if (nibble > ENC_NIBBLE_MAX || dir == 0) {
        return;
    }
    for (i = 0; i < 8; i++) {
        nib[i] = (uint8_t)((bcd[i / 2] >> ((i & 1u) ? 0 : 4)) & 0x0Fu);
    }
    d = (dir > 0) ? 1 : -1;
    i = nibble;
    while (1) {
        int8_t v = (int8_t)nib[i] + d;
        if (v > 9) {
            nib[i] = 0;
            d = 1;
            if (i == 0) {
                break;
            }
            i--;
            continue;
        }
        if (v < 0) {
            nib[i] = 9;
            d = -1;
            if (i == 0) {
                nib[i] = 0;
                break;
            }
            i--;
            continue;
        }
        nib[i] = (uint8_t)v;
        break;
    }
    for (i = 0; i < 4; i++) {
        bcd[i] = (uint8_t)((nib[i * 2] << 4) | nib[i * 2 + 1]);
    }
}

int8_t encoder_quad_step(uint8_t *prev, uint8_t ab)
{
    /* Gray: 00 -> 01 -> 11 -> 10 -> 00 is CW (+1) for many KY-040. */
    static const int8_t k_tab[16] = {
        0, 1, -1, 0, -1, 0, 0, 1, 1, 0, 0, -1, 0, -1, 1, 0
    };
    uint8_t idx;
    int8_t s;

    ab = (uint8_t)(ab & 3u);
    idx = (uint8_t)((*prev << 2) | ab);
    *prev = ab;
    s = k_tab[idx];
    return s;
}

void encoder_btn_init(encoder_btn_t *st, uint8_t released)
{
    st->stable = released ? 1u : 0u;
    st->armed = 0;
    st->long_done = 0;
    st->t_edge = 0;
    st->t_down = 0;
}

encoder_btn_ev_t encoder_btn_poll(encoder_btn_t *st, uint8_t released,
                                  uint32_t now_ms)
{
    uint8_t rel = released ? 1u : 0u;

    if (rel != st->stable) {
        if (st->t_edge == 0) {
            st->t_edge = now_ms;
        }
        if ((uint32_t)(now_ms - st->t_edge) < ENC_BTN_DEB_MS) {
            return ENC_BTN_NONE;
        }
        st->t_edge = 0;
        st->stable = rel;
        if (!rel) {
            st->armed = 1;
            st->long_done = 0;
            st->t_down = now_ms;
        } else if (st->armed && !st->long_done) {
            st->armed = 0;
            return ENC_BTN_SHORT;
        } else {
            st->armed = 0;
        }
        return ENC_BTN_NONE;
    }
    st->t_edge = 0;
    if (st->armed && !st->long_done && !st->stable &&
        (uint32_t)(now_ms - st->t_down) >= ENC_LONG_MS) {
        st->long_done = 1;
        return ENC_BTN_LONG;
    }
    return ENC_BTN_NONE;
}
