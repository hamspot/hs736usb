#include "proto_debounce.h"

void proto_debounce_init(proto_debounce_t *st, uint8_t level)
{
    st->last_irq_ms = 0;
    st->last_qual_ms = 0;
    st->last_level = level;
    st->dirty = 0;
}

void proto_debounce_irq(proto_debounce_t *st, uint32_t now_ms)
{
    st->last_irq_ms = now_ms;
    st->dirty = 1;
}

bool proto_debounce_poll(proto_debounce_t *st, uint32_t now_ms, uint8_t level,
                         bool *revert)
{
    *revert = false;
    if (st->dirty == 0) {
        return false;
    }
    if ((uint32_t)(now_ms - st->last_irq_ms) < PROTO_DEBOUNCE_MS) {
        return false;
    }
    st->dirty = 0;
    if (level == st->last_level) {
        return false;
    }
    if (st->last_qual_ms != 0 &&
        (uint32_t)(now_ms - st->last_qual_ms) < PROTO_REVERT_MS) {
        *revert = true;
        st->last_qual_ms = 0;
    } else {
        st->last_qual_ms = now_ms;
    }
    st->last_level = level;
    return true;
}
