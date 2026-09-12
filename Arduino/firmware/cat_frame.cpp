#include "cat_frame.h"

#include <string.h>

static uint8_t s_buf[5];
static volatile uint8_t s_n;
static uint32_t s_last_ms;
static bool s_have_time;

void cat_frame_init(void)
{
    cat_frame_reset();
}

void cat_frame_reset(void)
{
    s_n = 0;
    s_have_time = false;
}

bool cat_frame_feed(uint8_t byte, uint32_t now_ms)
{
    /*
     * Incomplete host frames older than CAT_FRAME_GAP_MS are dropped.
     * 250 ms is just above the FT-736 CAT inter-byte maximum (200 ms).
     */
    if (s_n > 0 && s_have_time && (uint32_t)(now_ms - s_last_ms) > CAT_FRAME_GAP_MS) {
        s_n = 0;
    }
    if (s_n >= 5) {
        s_n = 0;
    }
    s_buf[s_n++] = byte;
    s_last_ms = now_ms;
    s_have_time = true;
    return s_n == 5;
}

void cat_frame_take(uint8_t out[5])
{
    memcpy(out, s_buf, 5);
    s_n = 0;
}
