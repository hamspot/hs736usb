#include "radio_uart.h"
#include "cat_timing.h"

#include <AltSoftSerial.h>

/*
 * Inter-byte gap is required by the FT-736R CAT spec (50-200 ms between
 * bytes of a 5-byte block). The radio does not ACK ordinary writes, so
 * elapsed time after UART TX is the completion signal we have.
 */
static const uint16_t k_inter_byte_ms = CAT_INTER_BYTE_MS;

/* Uno/Nano: D8 RX, D9 TX (AltSoftSerial). */
static AltSoftSerial radio;

#define QSIZE 80
static uint8_t s_q[QSIZE];
static uint8_t s_head;
static uint8_t s_tail;
static uint8_t s_count;
static uint32_t s_last_tx_ms;
static bool s_ever_tx;

void radio_uart_begin(void)
{
    s_head = 0;
    s_tail = 0;
    s_count = 0;
    s_ever_tx = false;
    radio.begin(4800);
}

bool radio_uart_queue(const uint8_t blk[5])
{
    uint8_t i;

    if ((uint8_t)(s_count + 5) > QSIZE) {
        return false;
    }
    for (i = 0; i < 5; i++) {
        s_q[s_tail] = blk[i];
        s_tail = (uint8_t)((s_tail + 1) % QSIZE);
        s_count++;
    }
    return true;
}

void radio_uart_poll(uint32_t now_ms)
{
    if (s_count == 0) {
        return;
    }
    if (s_ever_tx && (uint32_t)(now_ms - s_last_tx_ms) < k_inter_byte_ms) {
        return;
    }
    radio.write(s_q[s_head]);
    s_head = (uint8_t)((s_head + 1) % QSIZE);
    s_count--;
    s_last_tx_ms = now_ms;
    s_ever_tx = true;
}

void radio_uart_discard_rx(void)
{
    while (radio.available() > 0) {
        (void)radio.read();
    }
}

int radio_uart_available(void)
{
    return radio.available();
}

int radio_uart_read(void)
{
    return radio.read();
}

bool radio_uart_busy(void)
{
    return s_count != 0;
}
