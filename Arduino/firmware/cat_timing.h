#ifndef CAT_TIMING_H
#define CAT_TIMING_H

/*
 * FT-736R CAT: 4800 8N2, 50–200 ms between bytes of a 5-byte block.
 * Byte time is ~2.3 ms; the gaps dominate.
 */
#define CAT_RADIO_BAUD 4800u
#define CAT_RADIO_FRAME_BITS 11u
#define CAT_INTER_BYTE_MS 50u
#define CAT_BLOCK_LEN 5u

#define CAT_BLOCK_UART_MS \
    (((CAT_BLOCK_LEN) * (CAT_RADIO_FRAME_BITS) * 1000u + (CAT_RADIO_BAUD) - 1u) / (CAT_RADIO_BAUD))
#define CAT_BLOCK_GAP_MS (((CAT_BLOCK_LEN) - 1u) * (CAT_INTER_BYTE_MS))
/* Time to shift out one F7 command (gaps + UART), not including the reply. */
#define CAT_SMETER_CMD_MS (CAT_BLOCK_GAP_MS + CAT_BLOCK_UART_MS)

#endif
