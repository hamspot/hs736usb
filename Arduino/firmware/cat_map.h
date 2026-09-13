#ifndef CAT_MAP_H
#define CAT_MAP_H

#include <stdint.h>
#include <stdbool.h>

#define CAT_BLOCK 5
#define CAT_MAX_RADIO_FRAMES 3
#define CAT_HOST_MAX 32
#define CAT_DEBUG_LEN 32
/* Host dump: A5 73 36 00 FE — not a Yaesu opcode. */
#define CAT_DEBUG_P0 0xA5
#define CAT_DEBUG_P1 0x73
#define CAT_DEBUG_P2 0x36
#define CAT_DEBUG_P3 0x00
#define CAT_DEBUG_OP 0xFE
/* Dialect override: A5 73 36 <sel> FC. sel 00=jumper 01=847 02=736 FF=cycle. */
#define CAT_SRC_OP 0xFC
#define CAT_SRC_SEL_JUMPER 0x00
#define CAT_SRC_SEL_847 0x01
#define CAT_SRC_SEL_736 0x02
#define CAT_SRC_SEL_CYCLE 0xFF

typedef enum {
    CAT_SRC_JUMPER = 0,
    CAT_SRC_847 = 1,
    CAT_SRC_736 = 2
} cat_src_t;

typedef struct {
    uint8_t n_radio;
    uint8_t radio[CAT_MAX_RADIO_FRAMES][CAT_BLOCK];
    uint8_t n_host;
    uint8_t host[CAT_HOST_MAX];
    uint8_t radio_reply_len; /* 5: wait for 736 E7/F7 and send to USB */
} cat_result_t;

#define CAT_BAND_50   0x01u
#define CAT_BAND_144  0x02u
#define CAT_BAND_220  0x04u
#define CAT_BAND_430  0x08u
#define CAT_BAND_1240 0x10u

typedef enum {
    CAT_PROTO_847 = 0,
    CAT_PROTO_736 = 1
} cat_proto_t;

void cat_map_init(void);
void cat_map_set_proto(cat_proto_t proto);
cat_proto_t cat_map_proto(void);
void cat_map_set_src(cat_src_t src);
cat_src_t cat_map_src(void);
/* ISR: latch A1. Returns true if the effective dialect changed (reset the USB frame). */
bool cat_map_note_jumper(cat_proto_t jumper);
void cat_map_dispatch(const uint8_t cmd[CAT_BLOCK], cat_result_t *out);

bool cat_is_on(void);
bool cat_ptt(void);
bool cat_sat(void);
uint8_t cat_bcd0_main(void);
uint8_t cat_bcd0_sat_rx(void);
uint8_t cat_bcd0_sat_tx(void);
uint8_t cat_band_from_bcd0(uint8_t b0);
uint8_t cat_band_mask(void);
void cat_map_debug_dump(uint8_t out[CAT_DEBUG_LEN]);
void cat_map_note_smeter(uint8_t raw, bool sql_closed);
void cat_copy_main(uint8_t out[CAT_BLOCK]);
void cat_copy_last_radio(uint8_t out[CAT_BLOCK]);
uint8_t cat_mode_main(void);
void cat_map_knob_freq(int8_t dir, uint8_t nibble, cat_result_t *out);
void cat_map_knob_mode(cat_result_t *out);

#endif
