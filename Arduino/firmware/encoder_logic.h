#ifndef ENCODER_LOGIC_H
#define ENCODER_LOGIC_H

#include <stdint.h>
#include <stdbool.h>

#define ENC_CURSOR_DEFAULT 0xFFu
#define ENC_NIBBLE_MAX 6u /* displayed digits; 10 Hz nibble is hidden */
#define ENC_LONG_MS 600u
#define ENC_BTN_DEB_MS 30u

/* AM / FM-N / FM-W: 1 kHz (nibble 5). Else 100 Hz (nibble 6). */
uint8_t encoder_default_nibble(uint8_t mode);
bool encoder_is_amfm(uint8_t mode);
uint8_t encoder_next_mode(uint8_t mode);
uint8_t encoder_cursor_next(uint8_t cursor);

/* Packed BCD freq[0..3], 10 Hz units. nibble 0 = 100 MHz … 6 = 100 Hz. */
void encoder_add_nibble(uint8_t bcd[4], uint8_t nibble, int8_t dir);

/* prev/ab are 2-bit Gray (B<<1|A). Returns -1, 0, +1 detents. */
int8_t encoder_quad_step(uint8_t *prev, uint8_t ab);

typedef enum {
    ENC_BTN_NONE = 0,
    ENC_BTN_SHORT,
    ENC_BTN_LONG
} encoder_btn_ev_t;

typedef struct {
    uint8_t stable; /* 1 = released (SW high) */
    uint8_t armed;
    uint8_t long_done;
    uint32_t t_edge;
    uint32_t t_down;
} encoder_btn_t;

void encoder_btn_init(encoder_btn_t *st, uint8_t released);
encoder_btn_ev_t encoder_btn_poll(encoder_btn_t *st, uint8_t released,
                                  uint32_t now_ms);

#endif
