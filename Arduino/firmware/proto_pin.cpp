#include "proto_pin.h"

#include <Arduino.h>
#include <avr/interrupt.h>

#include "cat_frame.h"
#include "cat_map.h"
#include "proto_debounce.h"

/* A1 / PC1 / PCINT9. INPUT_PULLUP: HIGH = 847, LOW = 736 native. */
static const uint8_t k_proto_pin = A1;
static proto_debounce_t s_deb;

static uint8_t pin_level(void)
{
    return ((PINC & _BV(PC1)) == 0) ? 0 : 1;
}

static cat_proto_t proto_from_level(uint8_t level)
{
    return (level == 0) ? CAT_PROTO_736 : CAT_PROTO_847;
}

static void apply_stable(uint8_t level, bool revert)
{
    cat_proto_t jumper;

    jumper = proto_from_level(level);
    if (revert) {
        cat_map_set_src(CAT_SRC_JUMPER);
    }
    if (cat_map_note_jumper(jumper) || revert) {
        cat_frame_reset();
    }
}

ISR(PCINT1_vect)
{
    proto_debounce_irq(&s_deb, millis());
}

void proto_pin_begin(void)
{
    uint8_t level;

    pinMode(k_proto_pin, INPUT_PULLUP);
    level = pin_level();
    proto_debounce_init(&s_deb, level);
    (void)cat_map_note_jumper(proto_from_level(level));
    PCMSK1 |= _BV(PCINT9);
    PCICR |= _BV(PCIE1);
}

void proto_pin_poll(uint32_t now_ms)
{
    bool revert;
    uint8_t level;

    level = pin_level();
    if (!proto_debounce_poll(&s_deb, now_ms, level, &revert)) {
        return;
    }
    apply_stable(level, revert);
}
