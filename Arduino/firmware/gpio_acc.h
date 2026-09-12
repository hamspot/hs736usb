#ifndef GPIO_ACC_H
#define GPIO_ACC_H

#include <stdint.h>
#include <stdbool.h>

void gpio_acc_begin(void);
void gpio_acc_poll(uint32_t now_ms);

#endif
