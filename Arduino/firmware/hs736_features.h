#ifndef HS736_FEATURES_H
#define HS736_FEATURES_H

/*
 * I2C expander for a bare HD44780 (no backpack) and KY-040 encoder.
 *
 *   HS736_MCP  0  = omit expander, LCD, encoder, I2C
 *              8  = MCP23008 (LCD 6 GPIO + encoder A/B; SW on Nano D12)
 *             17  = MCP23017 (LCD + encoder A/B/SW all on the chip)
 *
 * 4-bit HD44780 still needs RS, E, D4–D7 (six lines). The 08 has eight
 * GPIOs, so the shaft switch is D12. CAT/PTT/band/PWM/BUSY/LED/A1 stay.
 *
 * Compile examples:
 *   -DHS736_MCP=0
 *   -DHS736_MCP=8
 *   -DHS736_USE_MCP23017=0     (same as MCP=0)
 *   -DHS736_USE_MCP23008=1     (same as MCP=8)
 */
#ifndef HS736_MCP
#  if defined(HS736_USE_MCP23008) && (HS736_USE_MCP23008)
#    define HS736_MCP 8
#  elif defined(HS736_USE_MCP23017) && !(HS736_USE_MCP23017)
#    define HS736_MCP 0
#  else
#    define HS736_MCP 17
#  endif
#endif

#define HS736_USE_MCP (HS736_MCP != 0)

#endif
