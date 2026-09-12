#include "gpio_acc.h"

#include <Arduino.h>

#include "cat_map.h"
#include "gpio_logic.h"
#include "lcd_ui.h"
#include "radio_uart.h"

/* PTT MOSFET gate. HIGH = TX (low-side switch on). */
static const uint8_t k_ptt_out = 2;
/* S-meter PWM. Timer2 OC2B — not Timer1 (AltSoftSerial). */
static const uint8_t k_smeter_pwm = 3;
static const uint8_t k_hot_50 = 4;
static const uint8_t k_hot_144 = 5;
static const uint8_t k_hot_220 = 6;
static const uint8_t k_hot_430 = 7;
/* CAT pin 3 BUSY. INPUT_PULLUP; LOW = carrier / squelch open. */
static const uint8_t k_busy = 10;
static const uint8_t k_dialect_led = 13;
/*
 * Optional PTT sense from the radio RCA (MOX / mic / footswitch).
 * Active LOW after 22k + 5.1 V zener (see docs/WIRING.md). INPUT_PULLUP
 * so an unconnected jack reads RX.
 */
static const uint8_t k_ptt_in = A0;

static const uint8_t k_meter_cmd[5] = {0x00, 0x00, 0x00, 0x00, 0xF7};
static const uint16_t k_meter_reply_ms = 400;

enum {
    METER_IDLE = 0,
    METER_WAIT_TX,
    METER_WAIT_RX
};

static uint8_t s_meter_state;
static uint32_t s_last_poll_ms;
static uint32_t s_last_tx_ms;
static uint32_t s_rx_deadline_ms;
static uint8_t s_pwm;
static uint16_t s_shown_q8;
static uint32_t s_slew_last_ms;
static uint8_t s_rxn;
static uint8_t s_rxb[5];

static bool hw_ptt_low(void)
{
    return digitalRead(k_ptt_in) == LOW;
}

static bool keyed_now(void)
{
    return cat_ptt() || hw_ptt_low();
}

/* LOW on BUSY = squelch open (carrier). Floating pull-up = closed. */
static bool sql_closed_now(void)
{
    return digitalRead(k_busy) != LOW;
}

static void write_bands(uint8_t mask)
{
    digitalWrite(k_hot_50, (mask & CAT_BAND_50) ? HIGH : LOW);
    digitalWrite(k_hot_144, (mask & CAT_BAND_144) ? HIGH : LOW);
    digitalWrite(k_hot_220, (mask & CAT_BAND_220) ? HIGH : LOW);
    digitalWrite(k_hot_430, (mask & CAT_BAND_430) ? HIGH : LOW);
}

void gpio_acc_begin(void)
{
    pinMode(k_ptt_out, OUTPUT);
    digitalWrite(k_ptt_out, LOW);
    pinMode(k_smeter_pwm, OUTPUT);
    analogWrite(k_smeter_pwm, 0);
    pinMode(k_hot_50, OUTPUT);
    pinMode(k_hot_144, OUTPUT);
    pinMode(k_hot_220, OUTPUT);
    pinMode(k_hot_430, OUTPUT);
    pinMode(k_busy, INPUT_PULLUP);
    pinMode(k_dialect_led, OUTPUT);
    digitalWrite(k_dialect_led, HIGH);
    pinMode(k_ptt_in, INPUT_PULLUP);
    lcd_ui_begin();
    s_meter_state = METER_IDLE;
    s_last_poll_ms = 0;
    s_last_tx_ms = 0;
    s_pwm = 0;
    s_shown_q8 = 0;
    s_slew_last_ms = 0;
}

static void slew_pwm(uint32_t now_ms)
{
    uint16_t dt;
    uint16_t window;

    if (s_slew_last_ms == 0 || now_ms < s_slew_last_ms) {
        dt = 1;
    } else {
        dt = (uint16_t)(now_ms - s_slew_last_ms);
        if (dt == 0) {
            analogWrite(k_smeter_pwm, (uint8_t)(s_shown_q8 >> 8));
            return;
        }
    }
    s_slew_last_ms = now_ms;
    window = (uint16_t)gpio_meter_period_ms(now_ms, s_last_tx_ms, s_pwm);
    s_shown_q8 = gpio_smeter_slew_q8(s_shown_q8, s_pwm, dt, window);
    analogWrite(k_smeter_pwm, (uint8_t)(s_shown_q8 >> 8));
}

void gpio_acc_poll(uint32_t now_ms)
{
    bool keyed;
    uint8_t mask;
    uint32_t period;
    int b;

    keyed = keyed_now();
    digitalWrite(k_ptt_out, keyed ? HIGH : LOW);
    digitalWrite(k_dialect_led, cat_map_proto() == CAT_PROTO_847 ? HIGH : LOW);
    if (keyed) {
        s_last_tx_ms = now_ms;
    }

    mask = cat_band_mask();
    write_bands(mask);

    /*
     * 736 does not stream S-meter. Query F7 when CAT is on and the UART is
     * free. Fast period is 3× command-send time; PWM slews between samples.
     */
    if (!cat_is_on()) {
        s_pwm = 0;
        s_meter_state = METER_IDLE;
        radio_uart_discard_rx();
        slew_pwm(now_ms);
        lcd_ui_poll(now_ms, (uint8_t)(s_shown_q8 >> 8));
        return;
    }

    if (s_meter_state == METER_IDLE) {
        radio_uart_discard_rx();
        period = gpio_meter_period_ms(now_ms, s_last_tx_ms, s_pwm);
        if (!radio_uart_busy() &&
            (s_last_poll_ms == 0 || (uint32_t)(now_ms - s_last_poll_ms) >= period)) {
            if (radio_uart_queue(k_meter_cmd)) {
                s_meter_state = METER_WAIT_TX;
            }
        }
    } else if (s_meter_state == METER_WAIT_TX) {
        if (!radio_uart_busy()) {
            s_rxn = 0;
            s_rx_deadline_ms = now_ms + k_meter_reply_ms;
            s_meter_state = METER_WAIT_RX;
        }
    } else {
        while (radio_uart_available() > 0 && s_rxn < 5) {
            b = radio_uart_read();
            if (b < 0) {
                break;
            }
            s_rxb[s_rxn++] = (uint8_t)b;
        }
        if (s_rxn >= 5) {
            s_pwm = gpio_smeter_pwm(s_rxb[0]);
            cat_map_note_smeter(s_rxb[0], sql_closed_now());
            s_last_poll_ms = now_ms;
            s_meter_state = METER_IDLE;
        } else if ((int32_t)(now_ms - s_rx_deadline_ms) >= 0) {
            s_last_poll_ms = now_ms;
            s_meter_state = METER_IDLE;
        }
    }
    slew_pwm(now_ms);
    lcd_ui_poll(now_ms, (uint8_t)(s_shown_q8 >> 8));
}
