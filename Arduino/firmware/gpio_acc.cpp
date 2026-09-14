#include "gpio_acc.h"
#include "hs736_features.h"

#include <Arduino.h>
#if HS736_USE_MCP
#include <avr/interrupt.h>
#include <string.h>
#include "encoder_logic.h"
#include "mcp23017.h"
#endif

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
#if HS736_USE_MCP
/* MCP INT. D11 / PB3 / PCINT3. */
static const uint8_t k_mcp_int = 11;
#if HS736_MCP == 8
/* Shaft switch: 08 has no spare GPIO after LCD+A/B. D12 / PB4 / PCINT4. */
static const uint8_t k_enc_sw = 12;
#endif
#endif

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
#if HS736_USE_MCP
static volatile uint8_t s_enc_irq;
static uint8_t s_quad_prev;
static int8_t s_quad_acc;
static encoder_btn_t s_btn;
static uint8_t s_pend_n;
static uint8_t s_pend[CAT_MAX_RADIO_FRAMES][CAT_BLOCK];

ISR(PCINT0_vect)
{
    s_enc_irq = 1;
}

static void pend_frames(const cat_result_t *r)
{
    uint8_t i;

    for (i = 0; i < r->n_radio && s_pend_n < CAT_MAX_RADIO_FRAMES; i++) {
        memcpy(s_pend[s_pend_n], r->radio[i], CAT_BLOCK);
        s_pend_n++;
    }
}

static void flush_pend(void)
{
    while (s_pend_n > 0 && radio_uart_queue(s_pend[0])) {
        uint8_t i;
        s_pend_n--;
        for (i = 0; i < s_pend_n; i++) {
            memcpy(s_pend[i], s_pend[i + 1], CAT_BLOCK);
        }
    }
}

static uint8_t enc_nibble_now(void)
{
    uint8_t c;

    c = lcd_ui_cursor();
    if (c == ENC_CURSOR_DEFAULT) {
        return encoder_default_nibble(cat_mode_main());
    }
    return c;
}

static void encoder_service(uint32_t now_ms)
{
    uint8_t b;
    uint8_t ab;
    uint8_t rel;
    int8_t q;
    encoder_btn_ev_t ev;
    cat_result_t r;

    b = mcp23017_read_b();
    ab = (uint8_t)(((b & MCP_ENC_B) ? 2u : 0u) | ((b & MCP_ENC_A) ? 1u : 0u));
    q = encoder_quad_step(&s_quad_prev, ab);
    s_quad_acc = (int8_t)(s_quad_acc + q);
    if (s_quad_acc >= 4 || s_quad_acc <= -4) {
        cat_map_knob_freq((int8_t)(s_quad_acc > 0 ? 1 : -1), enc_nibble_now(), &r);
        pend_frames(&r);
        s_quad_acc = 0;
        lcd_ui_set_cursor(lcd_ui_cursor());
    }

    rel = (b & MCP_ENC_SW) ? 1u : 0u;
    ev = encoder_btn_poll(&s_btn, rel, now_ms);
    if (ev == ENC_BTN_SHORT) {
        lcd_ui_set_cursor(encoder_cursor_next(lcd_ui_cursor()));
    } else if (ev == ENC_BTN_LONG) {
        cat_map_knob_mode(&r);
        pend_frames(&r);
        lcd_ui_set_cursor(ENC_CURSOR_DEFAULT);
    }
}
#endif /* HS736_USE_MCP */

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
#if HS736_USE_MCP
    pinMode(k_mcp_int, INPUT_PULLUP);
    PCMSK0 |= _BV(PCINT3);
#if HS736_MCP == 8
    pinMode(k_enc_sw, INPUT_PULLUP);
    PCMSK0 |= _BV(PCINT4);
#endif
    PCICR |= _BV(PCIE0);
    lcd_ui_begin();
    s_quad_prev = (uint8_t)(mcp23017_read_b() & 3u);
    encoder_btn_init(&s_btn, 1);
    s_enc_irq = 1;
    s_quad_acc = 0;
    s_pend_n = 0;
#endif
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

#if HS736_USE_MCP
    encoder_service(now_ms);
    if (s_meter_state == METER_IDLE) {
        flush_pend();
    }
#endif

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
