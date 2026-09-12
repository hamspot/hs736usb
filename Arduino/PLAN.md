# HS-736: PIC18F14K50 → ATmega328 Arduino port

Port [hamspot/hs736usb](https://github.com/hamspot/hs736usb) so an ATmega328P Arduino (Uno/Nano class) sits between a PC and a Yaesu FT-736R: the PC talks **FT-847 CAT** (read/write, Hamlib/HRD/SatPC32), the box translates and drives the **FT-736R CAT** jack (mostly write-only frequency/mode; squelch and S-meter can be read).

Original firmware is the behavioral spec. [Hamlib](https://github.com/Hamlib/Hamlib) `rigs/yaesu/ft847.c` and `ft736.c` are the protocol reference (local copy: `/home/user/Documents/Hamlib`).

License: original CAT translator in `Firmware/Source/main.c` (`To736` / `ProcessIO`, Albert C. Jones / N6BIL, 2010) is **GPL-3**. This port is a derivative and stays GPL-3. Microchip’s USB stack in that file is **not** copied.

---

## What the PIC actually does

USB CDC (VID 0x04D8 / PID 0x000A, generic CDC) ↔ PIC UART TTL:

| PIC pin | Function |
| --- | --- |
| RB7 | UART TX → radio S-OUT pad |
| RB5 | UART RX ← radio S-IN pad |
| RC0/RC1 | LEDs |
| — | Radio UART **fixed 4800**, 8 data. Host USB baud is independent (CDC). |

`ProcessIO()` is the whole product (excerpt in `upstream/ProcessIO-excerpt.c`):

1. Collect host bytes into a 5-byte Yaesu block (`cmd[0..3]` params, `cmd[4]` opcode).
2. **Reads** (never go to the 736): `0x03` / `0x13` / `0x23` freq+mode, `0xE7` RX status, `0xF7` TX status — reply from RAM caches.
3. **Writes** translated to the 736, 5 bytes, wait for TX empty. PIC used **no** 50–200 ms inter-byte delay; this port does (CAT spec).
4. PIC forwarded UART bytes from the 736 to USB. v1 of this port does **not** splice 736 frames onto the 847 stream.

Caches default to 145.0000 MHz FM (`{0x14,0x50,0x00,0x00,0x08}`).

On **CAT ON** (`0x00`) the adapter also forces 145.0000 + FM onto the 736 so host and radio start aligned. CAT ON on the 736 **locks the front panel**.

---

## Protocol (Hamlib + PIC)

Both radios use 5-byte blocks, opcode last, 4800 8N2. FT-736 manual: **50–200 ms between bytes**. Hamlib: `write_delay` 50 ms (847) / 30 ms (736).

### Opcode map (host 847 → radio 736)

| Host opcode | Meaning (ft847.c) | Action |
| --- | --- | --- |
| `00` | CAT ON | Send as-is; then set cached main freq (`01`) and FM (`08 00 00 00 07`) |
| `80` | CAT OFF | Send as-is |
| `08` / `88` | PTT on / off | Send as-is; update TX/RX status cache |
| `01` | Set main freq | Cache 4 BCD bytes; 23 cm remap; send opcode `01` |
| `11` | Set SAT RX freq | Cache; remap opcode **`1E`** (736 full-duplex RX freq) |
| `21` | Set SAT TX freq | Cache; remap opcode **`2E`** |
| `03` / `13` / `23` | Get freq+mode main / sat RX / sat TX | Reply 5 bytes from cache |
| `07` / `17` / `27` | Set mode main / sat RX / sat TX | Forward; store mode in cache byte 4. 736 has no AM/CWR |
| `4E` | Sat mode ON | Remap to **`0E`** (736 “full duplex on”) |
| `8E` | Sat mode OFF | Send `8E`; restore main freq with `01` |
| `09` | Repeater shift (HRD/PIC) | `cmd[4] = cmd[0]` then send (`09` minus, `49` plus, `89` simplex) |
| `49` / `89` | Repeater plus / simplex (Hamlib 847) | Forward as-is. 736 simplex is **`89` not `88`** (`88` is PTT off) |
| `0A` | CTCSS/DCS mode | `2A` (847 ENC/DEC) → opcode `0A`; `0A` (847 DCS) → **drop** (PIC sent `00` = CAT ON; that is a bug); `4A`/`8A` forwarded as opcode |
| `0B` | CTCSS tone | Opcode **`FA`**; index PIC `Tone[38]`. If `cmd[0] > 31`, index `cmd[0]-26`. Clamp if out of range |
| `E7` / `F7` | Get RX / TX status | Reply from cache; do **not** forward as 847 status |
| `F9` | Repeater offset | Forward as-is (736 supports it; PIC dropped it) |
| `1A`/`2A`/`1B`/`2B`/`0C`/`1C`/`2C` | Sat CTCSS / DCS | Drop (no 736 DCS; sat CTCSS later) |

BCD frequency is 8 packed digits, **10 Hz units**, MSB first (Hamlib `to_bcd_be(cmd, freq/10, 8)`).

### 23 cm (1240 MHz)

FT-847 has no 4th MHz digit, so the host shows **240 MHz**. PIC: if `cmd[0]` is `0x24`..`0x30`, add `0xA0` on the **radio copy only** (`0x24` → `0xC4`). Cache returned to the PC stays 240 MHz.

Hamlib native 736 uses `(cmd[0] & 0x0f) | 0xc0` after encoding the true 1.24 GHz value (`0xC2 0x40 …` for 1240.0000). **v1 matches PIC**. Treat Hamlib’s encoding as a 23 cm bench check.

### Status reads

FT-736 **cannot** return frequency or mode. Hamlib `ft736_get_freq` is cache-only.

736 **can** return meters: after `E7`/`F7` it sends five bytes (four copies of squelch or S-meter plus echoed opcode).

FT-847 status is **one** byte:

- `E7` RX: bit7 = squelch closed (DCD off), bits0–4 = S-meter dots
- `F7` TX: bit7 = 1 means RX (`PTT_OFF`), bits0–4 = PO/ALC

v1 replies (Hamlib + HRD):

- `03`/`13`/`23`: 5-byte cache (BCD + mode)
- `E7`/`F7`: 5-byte buffer, **byte0 847-coded** (PIC PTT/squelch bits). Extra four bytes keep HRD happy; Hamlib reads 1 byte then flushes.

Unknown host opcodes: drop. Do not echo garbage to the 736.

### Framing

5-byte assembler: append host bytes; dispatch at 5; keep remainder. If a gap **> 250 ms**, discard partial frame (CAT inter-byte max is 200 ms — protocol timeout, not a poll loop).

---

## Bugs in the PIC firmware

Evidence is `upstream/ProcessIO-excerpt.c` (from hamspot `Firmware/Source/main.c`). These are defects in the translator, not “736 limitations.” This port fixes the ones marked **fixed**.

### 1. DCS ON is rewritten as CAT ON — **fixed**

```
cmd[4] = cmd[0];
if (cmd[0] == 0x2A) cmd[4] = 0x0A;   /* 847 ENC/DEC -> 736 TSQL */
if (cmd[0] == 0x0A) cmd[4] = 0x00;   /* 847 DCS ON  -> opcode 00 */
To736();
```

FT-847 DCS on is `{0x0A, 0x00, 0x00, 0x00, 0x0A}` (Hamlib `FT_847_NATIVE_CAT_SET_DCS_ON_MAIN`). The PIC copies byte 0 into the opcode, then the second `if` overwrites that opcode with **`0x00` (CAT ON)**. The 736 has no DCS; the harm is not “ignore DCS,” it is **re-asserting CAT**, which locks the front panel again. This port **drops** 847 DCS instead of sending `00`.

### 2. `cmd[10]` overflows on a 64-byte USB packet — **fixed**

`cmd` is 10 bytes. `getsUSBUSART(..., 64)` can return up to 64. The copy loop is `cmd[psn] = RS232_Out_Data[x]; ++psn` with no cap. One full CDC packet writes off the end of `cmd` (and into whatever follows in `.udata`). This port accumulates one byte at a time into a 5-byte frame buffer.

### 3. At most one CAT command per USB packet; leftover 5-byte commands stall — **fixed**

After copying the whole packet, it dispatches **once** (`if (psn > 4)`), then:

```
if (psn > 5)
    for (x = 0; x < (psn - 5); ++x) cmd[x] = cmd[x+5];
if (psn > 4) psn = psn - 5;
RS232_Out_Data_Rdy = 0;
```

A 10-byte packet (two Yaesu blocks) runs the first command, rotates the second into `cmd[0..4]` with `psn == 5`, then **clears the “data ready” flag**. The second command sits until **another** USB packet arrives and the copy loop runs again. Hamlib usually sends one 5-byte block per write, so this often works; HRD bursts and back-to-back `rigctl` can stall a command. This port dispatches every complete 5-byte frame in the same `loop()`.

### 4. CTCSS `Tone[x]` has no range check — **fixed**

```
if (cmd[0] > 31) x = cmd[0] - 26;
else x = cmd[0];
cmd[0] = Tone[x];   /* Tone[38] */
```

Any host byte `>= 0x40` (64) yields `x >= 38` and reads off the end of `Tone[]`. This port drops the command if the index is out of range.

Related mismatch (not a bounds bug): the PIC treats byte 0 as an **HRD-style index** (0…31, then skip 26). Hamlib `ft847_set_ctcss_tone` sends **Yaesu CAT tone codes** (`0x3F`, `0x39`, … `0x00`), not 0-based indices. v1 keeps the PIC table so HRD behavior is unchanged; Hamlib CTCSS frequencies will still be wrong until a later Hamlib-code map is added. That is documented, not silently “fixed.”

### 5. No 50–200 ms inter-byte gap on the radio UART — **fixed**

`To736()` waits only for `TXSTA` shift-complete, then sends the next byte immediately. The FT-736R manual requires **50–200 ms between bytes** of a command block. Hamlib uses `write_delay` 30–50 ms for the same reason. Field units often still keyed the radio; this port enforces 50 ms because the spec says so, not because we guessed.

### 6. Radio RX is spliced onto the FT-847 USB stream

```
if (mDataRdyUSART()) {
    USB_Out_Buffer[NextUSBOut] = getcUSART();
    ++NextUSBOut;
    USB_Out_Buffer[NextUSBOut] = 0;
}
```

Any byte the 736 emits (squelch/S-meter replies, AQS, noise) is forwarded to the PC **in the middle of the 847 session**. Hamlib 847 then sees extra bytes before/after a 1-byte `E7` or a 5-byte `03` and can desynchronize. `NextUSBOut` is also incremented with **no cap** against `USB_Out_Buffer[]`. This port **discards** radio RX in v1 (freq/mode are cached; we do not query 736 `E7`/`F7` yet).

### 7. Hamlib repeater-shift opcodes `49` / `89` are ignored

PIC only handles opcode **`09`**, then replaces it with `cmd[0]` (HRD: minus/plus/simplex in byte 0). Hamlib 847 sends the shift **as the opcode**: `09` minus, `49` plus, `89` simplex (`ft847_set_rptr_shift`). Plus and simplex therefore never reach the 736. If HRD ever put **`88`** in byte 0 (736 manual typo for simplex; Hamlib 736 notes that `88` is PTT off), the PIC would send PTT off. This port forwards Hamlib `49`/`89` as-is and keeps the PIC `09`+param path.

### 8. CAT ON / sat-off clobber `cmd[]` after `To736()`

CAT ON sends the host `00` block, then **overwrites** `cmd[]` with cached freq and FM and sends those too. That is intended. Sat-off (`8E`) does the same for a follow-up `01`. Harmless if `To736` is synchronous (it is). Noted so the port’s three-frame result struct is not mistaken for a PIC bug.

### What we did not treat as bugs

- Caching freq/mode instead of reading the 736 — the radio cannot return them.
- Dummy `E7`/`F7` 5-byte replies — design for HRD; byte 0 happens to match 847 PTT/squelch bits.
- Dropping memory, AQS, DCS, sat CTCSS `1A`/`2A` — original README / no 736 feature.
- 23 cm `+ 0xA0` vs Hamlib `(bcd & 0x0f) | 0xc0` — two encodings; PIC’s is what HS-736USB shipped. v1 keeps PIC; bench on a 1.2 GHz module before changing.

---

## Hardware (ATmega328P)

The 328 has **one** hardware USART. On Uno/Nano that USART is the USB-serial bridge to the PC.

See `docs/WIRING.md` (Arduino, including accessory GPIO) and `docs/CONNECTIONS.md` (every FT-736R jack, with diagrams; CAT/DATA manuals are often wrong). Host settings: **Yaesu FT-847, 4800, 8N2, no handshake**.

Accessory GPIO (not 847 CAT): D2 PTT MOSFET (CAT PTT **or** A0 RCA sense, active low), D4–D7/D10 one-hot bands, D11/D12 2-bit band, D3 S-meter PWM from real 736 `F7` polls. **A1** (PCINT): HIGH = USB speaks FT-847, LOW = native FT-736; IRQ switches immediately without CAT-reset. Native 736 sat opcodes (`0E`/`1E`/`2E`) work only in 736 mode; 847 sat (`4E`/`11`/`21`) works only in 847 mode. See `docs/PROTOCOLS.md`.

---

## Firmware layout

```
PLAN.md
README.md
LICENSE
upstream/                 # original README, LICENSE, ProcessIO excerpt
firmware/                 # Arduino sketch (folder name = sketch name)
  firmware.ino
  cat_frame.{h,cpp}
  cat_map.{h,cpp}
  radio_uart.{h,cpp}
docs/WIRING.md
docs/CONNECTIONS.md          # FT-736R front/rear pinouts + diagrams
docs/CAT.md
test/golden_vectors.py
test/test_cat_map.c
test/README.md
```

`loop()`:

1. Drain `Serial` into the frame assembler; on complete frame, dispatch.
2. Reads: write reply to USB.
3. Writes: queue radio bytes with 50 ms between bytes (FT-736 CAT spec).
4. Drain radio RX; v1 discards. Do not splice 736 frames onto the 847 stream.

---

## Testing

Host (no Arduino): `test/test_cat_map.c` + `test/golden_vectors.py`.

On hardware without the 736: `rigctl -m 1001 -r /dev/ttyUSB0 -s 4800` (Hamlib FT-847). After `F`/`M`, `f`/`m` should return the cache; `t` should not time out.

On the radio: CAT LED, VFO + sat VFO as in the original README.

---

## Out of scope (v1)

- Memory channels
- AQS / digital messages
- DCS
- Replacing Hamlib’s FT-736 backend (this box **is** an 847)
- Custom USB VID/PID
- Mega / 32U4 / ESP32
- New PCB

---

## Key decisions

| Decision | Why |
| --- | --- |
| Pretend 847 to the PC, 736 to the radio | Same product as HS-736USB; Hamlib 847 (`-m 1001`) has sat VFOs and reads |
| Faithful PIC opcode map, plus `F9` and Hamlib `49`/`89` | Known HRD behavior; Hamlib 736 confirms `1E`/`2E`/`0E`/`FA` |
| Cache freq/mode; 5-byte `E7`/`F7` with 847-coded byte0 | 736 has no freq read; HRD wanted 5 bytes; Hamlib 847 reads 1 byte then flushes |
| Hardware USART = PC, AltSoftSerial 8/9 = radio | Only one USART on 328; USB is the busy polling side |
| Host locked to 4800 8N2 | Uno UART baud is the CDC baud; 847/736 native rate |
| 50 ms radio inter-byte delay | FT-736 CAT spec 50–200 ms; PIC omitted it |
| Drop 847 DCS (`0A`/`0A`) instead of sending CAT ON | PIC bug: it rewrote the opcode to `00` |
| GPL-3, no Microchip USB sources | Translator is GPL; USB stack is not ours |
