#ifndef CAT_FRAME_H
#define CAT_FRAME_H

#include <stdint.h>
#include <stdbool.h>

#define CAT_FRAME_GAP_MS 250

void cat_frame_init(void);
void cat_frame_reset(void);
bool cat_frame_feed(uint8_t byte, uint32_t now_ms);
void cat_frame_take(uint8_t out[5]);

#endif
