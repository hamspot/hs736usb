# Arduino / ATmega328P port

Drop-in replacement for the PIC18F14K50 HS-736USB on an Arduino Uno or Nano: the PC speaks **Yaesu FT-847 CAT** (or native **FT-736R** CAT); the box drives the FT-736R CAT jack at 4800 8N2 TTL.

The original PIC firmware, Eagle files, and manual stay in `Firmware/`, `Hardware/`, and `Manual/` at the repo root. This directory is GPL-3 like the rest of the project (N6BIL translator in `Firmware/Source/main.c`).

## Build

```
arduino-cli compile --fqbn arduino:avr:nano Arduino/firmware
arduino-cli upload --fqbn arduino:avr:nano -p /dev/ttyUSB0 Arduino/firmware
```

From this directory, `firmware` is enough. Libraries: **AltSoftSerial**, **Wire** (MCP23017 I2C). LCD is a bare HD44780 on the expander, not an I2C backpack. Set `HS736_USE_MCP23017` to **0** in `firmware/hs736_features.h` (or `-DHS736_USE_MCP23017=0`) to omit LCD, encoder, and I2C.

Host: **4800 8N2**, no handshake. Default dialect is FT-847 (Hamlib `-m 1001`). Pin **A1** low, or CAT `A5 73 36 02 FC`, selects native 736 (`-m 1010`).

- Operating manual (PDF, with the PIC booklet): `../Manual/HS-736USB-Operating-Manual.pdf`
- Wiring: `docs/WIRING.md`
- 847 vs 736 dialects: `docs/PROTOCOLS.md`
- Radio connectors (manual errors): `docs/CONNECTIONS.md`
- Design notes: `PLAN.md`

```
Arduino/test/run_suite.sh
Arduino/test/run_suite.sh /dev/ttyUSB0
```

Run the suite from the `Arduino/` directory (paths are relative to that folder).
