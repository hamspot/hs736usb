#ifndef GPIO_LOGIC_H
#define GPIO_LOGIC_H

#include <stdint.h>
#include <stdbool.h>
#include "cat_timing.h"

#define GPIO_SMETER_RAW_MIN 0x30u
#define GPIO_SMETER_RAW_MAX 0xADu
#define GPIO_SMETER_ACTIVE_PWM 84u /* 33% of 0..255 */
#define GPIO_POLL_IDLE_MS 3000u
/* Three command-send times between samples; PWM slews across that window. */
#define GPIO_POLL_ACTIVE_MS (3u * CAT_SMETER_CMD_MS)
#define GPIO_TX_HOLD_MS 300000u /* 5 minutes */

uint8_t gpio_smeter_pwm(uint8_t raw);
uint16_t gpio_smeter_slew_q8(uint16_t shown_q8, uint8_t target, uint16_t dt_ms,
                             uint16_t window_ms);
uint32_t gpio_meter_period_ms(uint32_t now_ms, uint32_t last_tx_ms, uint8_t pwm);
uint8_t gpio_binary_code(uint8_t mask, bool keyed, bool sat,
                         uint8_t bcd0_main, uint8_t bcd0_rx, uint8_t bcd0_tx);

#endif
