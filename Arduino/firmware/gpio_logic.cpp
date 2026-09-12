#include "gpio_logic.h"

uint8_t gpio_smeter_pwm(uint8_t raw)
{
    uint16_t span;
    uint16_t v;

    if (raw <= GPIO_SMETER_RAW_MIN) {
        return 0;
    }
    if (raw >= GPIO_SMETER_RAW_MAX) {
        return 255;
    }
    span = (uint16_t)(GPIO_SMETER_RAW_MAX - GPIO_SMETER_RAW_MIN);
    v = (uint16_t)(raw - GPIO_SMETER_RAW_MIN);
    return (uint8_t)((v * 255u) / span);
}

uint8_t gpio_smeter_dots(uint8_t raw)
{
    uint16_t span;
    uint16_t v;

    if (raw <= GPIO_SMETER_RAW_MIN) {
        return 0;
    }
    if (raw >= GPIO_SMETER_RAW_MAX) {
        return 31;
    }
    span = (uint16_t)(GPIO_SMETER_RAW_MAX - GPIO_SMETER_RAW_MIN);
    v = (uint16_t)(raw - GPIO_SMETER_RAW_MIN);
    return (uint8_t)((v * 31u) / span);
}

uint8_t gpio_smeter_rx_status(uint8_t raw, bool sql_closed)
{
    uint8_t b;

    b = gpio_smeter_dots(raw) & 0x1Fu;
    if (sql_closed) {
        b = (uint8_t)(b | 0x80u);
    }
    return b;
}

uint16_t gpio_smeter_slew_q8(uint16_t shown_q8, uint8_t target, uint16_t dt_ms,
                             uint16_t window_ms)
{
    uint16_t tgt_q8;
    int32_t diff;
    int32_t step;
    int32_t next;

    tgt_q8 = (uint16_t)((uint16_t)target << 8);
    if (shown_q8 == tgt_q8) {
        return shown_q8;
    }
    if (window_ms == 0 || dt_ms >= window_ms) {
        return tgt_q8;
    }
    if (dt_ms == 0) {
        return shown_q8;
    }
    diff = (int32_t)tgt_q8 - (int32_t)shown_q8;
    step = (diff * (int32_t)dt_ms) / (int32_t)window_ms;
    if (step == 0) {
        step = (diff > 0) ? 1 : -1;
    }
    next = (int32_t)shown_q8 + step;
    if (diff > 0 && next > (int32_t)tgt_q8) {
        next = tgt_q8;
    }
    if (diff < 0 && next < (int32_t)tgt_q8) {
        next = tgt_q8;
    }
    if (next < 0) {
        next = 0;
    }
    if (next > 65535) {
        next = 65535;
    }
    return (uint16_t)next;
}

uint32_t gpio_meter_period_ms(uint32_t now_ms, uint32_t last_tx_ms, uint8_t pwm)
{
    uint32_t since_tx;

    if (pwm >= GPIO_SMETER_ACTIVE_PWM) {
        return GPIO_POLL_ACTIVE_MS;
    }
    if (last_tx_ms != 0) {
        since_tx = (uint32_t)(now_ms - last_tx_ms);
        if (since_tx < GPIO_TX_HOLD_MS) {
            return GPIO_POLL_ACTIVE_MS;
        }
    }
    return GPIO_POLL_IDLE_MS;
}
