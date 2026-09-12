# Wiring (ATmega328P Uno / Nano)

Host settings in Hamlib, HRD, SatPC32, WSJT-X: **Yaesu FT-847**, **4800 baud**, **8 data bits**, **no parity**, **2 stop bits**, **no handshake**.

The Arduino USB-serial baud **must** match (this firmware uses 4800 8N2). The original PIC CDC ignored host baud; Uno/Nano cannot.

## CAT jack (FT-736R 6-pin DIN)

Full radio pinouts (front and rear, including known **manual errors**) are in [`CONNECTIONS.md`](CONNECTIONS.md).

AMSAT-corrected CAT map. Some Yaesu manuals number these pins wrong.

| DIN pin | Signal | Arduino |
| --- | --- | --- |
| 1 | GND | GND |
| 2 | Serial In (S.IN) — **into the radio** | **D9** (AltSoftSerial TX) |
| 3 | Busy | **D10** (LOW = squelch open) |
| 4 | Serial Out (S.OUT) — **out of the radio** | **D8** (AltSoftSerial RX) |
| 5 | NC | no connect |
| 6 | +13.8 V | **do not connect** |

Rear of **male** plug (pins toward you). Numbers are **not** sequential:

```
             3           1
                   6
             5           4
                   2

        1 GND     2 S.IN (adapter TX)     3 BUSY
        4 S.OUT (adapter RX)   5 NC    6 +13.8 V (leave open)
```

```
PC USB -- 328 USART0 (4800 8N2)     host sees an FT-847
D9 TX  ---------------->  CAT pin 2  S.IN     TTL, idle high
D8 RX  <----------------  CAT pin 4  S.OUT
GND    -----------------  CAT pin 1
D13        activity LED
         (no wire to pin 6)
```

Levels match the original HS-736USB (PIC UART TTL, no MAX232). If a particular radio only clocks inverted/open-collector CAT, add an NPN inverter on D9; that is not the default.

Power the Arduino from USB. Do not feed CAT pin 6 into the 5 V pin.

## Accessory GPIO (not FT-847 CAT)

PTT out, band decode, and S-meter PWM. See `CONNECTIONS.md` for the radio’s own STBY DIN (prefer that for band-keyed linears if you can).

| Pin | Direction | Function |
| --- | --- | --- |
| **D2** | out | PTT MOSFET gate. HIGH when **CAT PTT or RCA sense** is keyed |
| **D3** | PWM | S-meter (0–255), Timer2 |
| **D4** | out | One-hot 50 MHz |
| **D5** | out | One-hot 144 MHz |
| **D6** | out | One-hot 220 MHz |
| **D7** | out | One-hot 430 MHz |
| **D10** | in | CAT pin 3 **BUSY**. INPUT_PULLUP; **LOW = squelch open / carrier**. Measure 5 V before wiring. |
| **D11** | out | HD44780 RS |
| **D12** | out | HD44780 E (tie RW to GND) |
| **D13** | out | Dialect LED: HIGH = FT-847, LOW = FT-736 |
| **A0** | in | Optional PTT **sense** (active LOW) |
| **A1** | in (PCINT) | Protocol: **HIGH = FT-847**, **LOW = FT-736 native**. Debounced 50 ms. Two stable flips in 1 s restore jumper-follow if CAT had forced a dialect. See `docs/PROTOCOLS.md` |
| **A2–A5** | out | HD44780 D4–D7 (4-bit data) |

No 2-bit band outputs. 1240 MHz is on the LCD and in CAT (host shows 240 MHz), not a one-hot pin. Satellite: one-hot **both** RX and TX among 50/144/220/430.

16×2 Hitachi: line 1 frequency + mode + 847/736; line 2 last 5 radio bytes (hex) + 5-cell S-meter bar. Host `E7` returns the last polled 736 `F7` mapped to 847 RX status (bits 0–4 dots, bit7 squelch from BUSY).

### PTT MOSFET (output)

```
  D2 -- 100Ω -- MOSFET gate   (2N7000 small; IRLZ44 / logic-level for bigger coils)
                 10k to GND
  MOSFET source -- Arduino GND -- radio GND
  MOSFET drain  -- amp PTT / relay (coil other side to +12 V, diode across coil)
```

Output is HIGH for TX. That turns the N-MOSFET on and **grounds** the amp PTT line (same sense as the 736 STBY jack).

CAT `08`/`88` **or** the sense input keys this pin. Mic / MOX / footswitch therefore keys the amp even when CAT is off.

### PTT RCA sense (optional input)

The 736 PTT RCA is **8 V** open, ground-to-TX. **Do not** wire 8 V straight to A0.

Y-cable the radio RCA: one leg still to a footswitch if you use one; the other to the Arduino:

```
  RCA center -- 22 kΩ --+-- A0
                        +-- 5.1 V zener to GND   (cathode at A0)
  RCA sleeve ----------- GND

  Firmware: INPUT_PULLUP, TX = pin LOW.
  Unplugged = RX (pull-up).
```

Pass-through: `PTT_out = CAT_PTT OR (A0 == LOW)`.

Front-panel MOX and the mic PTT close the same RCA line, so this input sees them. CAT PTT keys the radio over serial, not the RCA; the MOSFET still follows CAT PTT.

### S-meter PWM

Queries the 736 with CAT `F7` (not the fake 847 `F7` on USB). **3 s** when idle; **3× command-send time** (~633 ms) when keyed in the last 5 minutes or PWM ≥ 84 (33%). D3 PWM **slews** from the last sample to the new one over that same window so the needle does not jump. Host CAT still wins the UART.

## Radio setup

From the original HS-736USB README: memory channels are not supported. Set the radio to **VFO**, and satellite mode to **VFO**, before enabling CAT. CAT ON locks the front panel. 1240 MHz is shown to the PC as 240 MHz.
