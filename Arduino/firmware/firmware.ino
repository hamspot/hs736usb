/*
 * HS-736 — FT-847 (USB) to FT-736R (TTL CAT) translator for ATmega328P.
 *
 * Derivative of hamspot/hs736usb ProcessIO (N6BIL), GPL-3.
 * See PLAN.md and docs/CAT.md.
 */

#include "cat_frame.h"
#include "cat_map.h"
#include "radio_uart.h"
#include "gpio_acc.h"
#include "proto_pin.h"

static const uint8_t k_led_pin = 13;
static uint32_t s_led_until_ms;
static uint8_t s_fwd_expect;
static uint8_t s_fwd_n;
static uint8_t s_fwd[5];
static uint32_t s_fwd_deadline;

void setup()
{
    pinMode(k_led_pin, OUTPUT);
    digitalWrite(k_led_pin, LOW);

    Serial.begin(4800, SERIAL_8N2);
    cat_frame_init();
    cat_map_init();
    radio_uart_begin();
    gpio_acc_begin();
    proto_pin_begin();
}

void loop()
{
    uint32_t now = millis();
    uint8_t cmd[5];
    cat_result_t result;
    uint8_t i;
    bool queued;
    int b;

    while (Serial.available() > 0) {
        if (!cat_frame_feed((uint8_t)Serial.read(), now)) {
            continue;
        }
        cat_frame_take(cmd);
        cat_map_dispatch(cmd, &result);

        if (result.n_host > 0) {
            Serial.write(result.host, result.n_host);
        }

        queued = true;
        for (i = 0; i < result.n_radio; i++) {
            if (!radio_uart_queue(result.radio[i])) {
                queued = false;
                break;
            }
        }
        if (result.radio_reply_len > 0) {
            s_fwd_expect = result.radio_reply_len;
            s_fwd_n = 0;
            s_fwd_deadline = now + 800;
        }
        digitalWrite(k_led_pin, HIGH);
        s_led_until_ms = now + (queued ? 40 : 200);
    }

    proto_pin_poll(now);
    radio_uart_poll(now);

    if (s_fwd_expect > 0) {
        if (!radio_uart_busy()) {
            while (radio_uart_available() > 0 && s_fwd_n < s_fwd_expect) {
                b = radio_uart_read();
                if (b < 0) {
                    break;
                }
                s_fwd[s_fwd_n++] = (uint8_t)b;
            }
            if (s_fwd_n >= s_fwd_expect) {
                Serial.write(s_fwd, s_fwd_expect);
                s_fwd_expect = 0;
            } else if ((int32_t)(now - s_fwd_deadline) >= 0) {
                s_fwd_expect = 0;
            }
        }
    } else {
        gpio_acc_poll(now);
    }

    if (s_led_until_ms != 0 && (int32_t)(now - s_led_until_ms) >= 0 && !radio_uart_busy()) {
        digitalWrite(k_led_pin, LOW);
        s_led_until_ms = 0;
    }
}
