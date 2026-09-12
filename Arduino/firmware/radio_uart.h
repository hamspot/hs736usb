#ifndef RADIO_UART_H
#define RADIO_UART_H

#include <stdint.h>
#include <stdbool.h>

void radio_uart_begin(void);
bool radio_uart_queue(const uint8_t blk[5]);
void radio_uart_poll(uint32_t now_ms);
void radio_uart_discard_rx(void);
int radio_uart_available(void);
int radio_uart_read(void);
bool radio_uart_busy(void);

#endif
