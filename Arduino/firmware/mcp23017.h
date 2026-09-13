#ifndef MCP23017_H
#define MCP23017_H

#include <stdint.h>
#include <stdbool.h>

#define MCP23017_ADDR 0x20

/* GPA0–3 LCD D4–D7, GPA4 RS, GPA5 E */
#define MCP_LCD_D4 0x01u
#define MCP_LCD_RS 0x10u
#define MCP_LCD_E 0x20u
#define MCP_ENC_A 0x01u
#define MCP_ENC_B 0x02u
#define MCP_ENC_SW 0x04u

bool mcp23017_begin(void);
void mcp23017_write_a(uint8_t v);
uint8_t mcp23017_read_b(void);

#endif
