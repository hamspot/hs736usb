#ifndef PROTO_DEBOUNCE_H
#define PROTO_DEBOUNCE_H

#include <stdint.h>
#include <stdbool.h>

/*
 * A1 toggle debounce + double-flip revert.
 * Bounce is mechanical; 50 ms quiet is required before a level counts.
 * Two qualified edges within 1 s (after debounce) return CAT source to jumper.
 * That window is a human gesture, not a radio protocol timeout.
 */
#define PROTO_DEBOUNCE_MS 50u
#define PROTO_REVERT_MS 1000u

typedef struct {
    uint32_t last_irq_ms;
    uint32_t last_qual_ms;
    uint8_t last_level;
    uint8_t dirty;
} proto_debounce_t;

void proto_debounce_init(proto_debounce_t *st, uint8_t level);
void proto_debounce_irq(proto_debounce_t *st, uint32_t now_ms);
bool proto_debounce_poll(proto_debounce_t *st, uint32_t now_ms, uint8_t level,
                         bool *revert);

#endif
