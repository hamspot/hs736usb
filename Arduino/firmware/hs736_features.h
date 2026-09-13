#ifndef HS736_FEATURES_H
#define HS736_FEATURES_H

/*
 * MCP23017 on I2C (A4/A5): parallel HD44780 + rotary encoder.
 * Set to 0 to omit the chip and everything on it (LCD, encoder, I2C).
 * CAT, PTT, band D4–D7, PWM, BUSY, dialect LED, A1 jumper stay.
 *
 * Override at compile:
 *   arduino-cli compile --fqbn arduino:avr:nano \
 *     --build-property compiler.cpp.extra_flags=-DHS736_USE_MCP23017=0 \
 *     Arduino/firmware
 */
#ifndef HS736_USE_MCP23017
#define HS736_USE_MCP23017 1
#endif

#endif
