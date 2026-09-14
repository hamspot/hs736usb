#include "mcp23017.h"

#if defined(ARDUINO) && HS736_USE_MCP
#include <Arduino.h>
#include <Wire.h>

#if HS736_MCP == 8
#define REG_IODIR 0x00
#define REG_GPINTEN 0x02
#define REG_INTCON 0x04
#define REG_GPPU 0x06
#define REG_GPIO 0x09
#define ENC_DIR 0xC0u /* GP6 A, GP7 B inputs */
#else
#define REG_IODIRA 0x00
#define REG_IODIRB 0x01
#define REG_GPINTENB 0x05
#define REG_INTCONB 0x09
#define REG_GPPUB 0x0D
#define REG_GPIOA 0x12
#define REG_GPIOB 0x13
#endif

static bool s_ok;

static bool wr(uint8_t reg, uint8_t v)
{
    Wire.beginTransmission(MCP23017_ADDR);
    Wire.write(reg);
    Wire.write(v);
    return Wire.endTransmission() == 0;
}

static bool rd(uint8_t reg, uint8_t *v)
{
    int n;

    Wire.beginTransmission(MCP23017_ADDR);
    Wire.write(reg);
    if (Wire.endTransmission(false) != 0) {
        return false;
    }
    n = Wire.requestFrom((uint8_t)MCP23017_ADDR, (uint8_t)1);
    if (n < 1) {
        return false;
    }
    *v = (uint8_t)Wire.read();
    return true;
}

bool mcp23017_begin(void)
{
    Wire.begin();
#if HS736_MCP == 8
    /* GP0–5 LCD out; GP6–7 encoder A/B in + pull-up + change INT. */
    s_ok = wr(REG_IODIR, ENC_DIR) && wr(REG_GPPU, ENC_DIR) && wr(REG_GPIO, 0) &&
           wr(REG_INTCON, 0) && wr(REG_GPINTEN, ENC_DIR);
#else
    s_ok = wr(REG_IODIRA, 0x00) && wr(REG_IODIRB, 0x07) && wr(REG_GPPUB, 0x07) &&
           wr(REG_GPIOA, 0x00) && wr(REG_INTCONB, 0x00) && wr(REG_GPINTENB, 0x03);
#endif
    return s_ok;
}

void mcp23017_write_a(uint8_t v)
{
    if (!s_ok) {
        return;
    }
#if HS736_MCP == 8
    (void)wr(REG_GPIO, (uint8_t)(v & 0x3Fu));
#else
    (void)wr(REG_GPIOA, v);
#endif
}

uint8_t mcp23017_read_b(void)
{
    uint8_t v;
    uint8_t enc;

    if (!s_ok) {
        return 0x07;
    }
#if HS736_MCP == 8
    if (!rd(REG_GPIO, &v)) {
        return 0x07;
    }
    enc = 0;
    if (v & 0x40u) {
        enc |= MCP_ENC_A;
    }
    if (v & 0x80u) {
        enc |= MCP_ENC_B;
    }
    /* SW is Nano D12 (PB4); high = released. */
    if (PINB & _BV(PB4)) {
        enc |= MCP_ENC_SW;
    }
    return enc;
#else
    if (!rd(REG_GPIOB, &v)) {
        return 0x07;
    }
    return v;
#endif
}

#else /* host tests or HS736_MCP=0 */

bool mcp23017_begin(void)
{
    return false;
}

void mcp23017_write_a(uint8_t v)
{
    (void)v;
}

uint8_t mcp23017_read_b(void)
{
    return 0x07;
}

#endif
