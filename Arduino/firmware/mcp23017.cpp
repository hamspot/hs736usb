#include "mcp23017.h"
#include "hs736_features.h"

#if defined(ARDUINO) && HS736_USE_MCP23017
#include <Arduino.h>
#include <Wire.h>

#define REG_IODIRA 0x00
#define REG_IODIRB 0x01
#define REG_GPINTENB 0x05
#define REG_INTCONB 0x09
#define REG_GPPUB 0x0D
#define REG_GPIOA 0x12
#define REG_GPIOB 0x13
#define REG_OLATA 0x14

static bool s_ok;
static uint8_t s_olat_a;

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
    s_olat_a = 0;
    /* GPA all outputs (LCD). GPB0–2 inputs with pull-ups (encoder). */
    s_ok = wr(REG_IODIRA, 0x00) && wr(REG_IODIRB, 0x07) && wr(REG_GPPUB, 0x07) &&
           wr(REG_OLATA, 0x00) && wr(REG_INTCONB, 0x00) && wr(REG_GPINTENB, 0x03);
    return s_ok;
}

void mcp23017_write_a(uint8_t v)
{
    s_olat_a = v;
    if (s_ok) {
        (void)wr(REG_GPIOA, v);
    }
}

uint8_t mcp23017_read_b(void)
{
    uint8_t v;

    if (!s_ok || !rd(REG_GPIOB, &v)) {
        return 0x07; /* idle: A/B/SW released high */
    }
    return v;
}

#else /* host tests or HS736_USE_MCP23017=0 */

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
