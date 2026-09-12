# Talking to HS-736: FT-847 vs FT-736R CAT

The adapter sits on USB as a 4800 8N2 CAT port. **Pin A1** (IRQ) selects which language the PC is speaking. It does **not** reset CAT on the radio.

| A1 | Protocol | PC software |
| --- | --- | --- |
| HIGH (open, pull-up) | **FT-847** (default) | Hamlib `-m 1001`, HRD, SatPC32 as FT-847 |
| LOW (jumper to GND) | **FT-736R native** | Hamlib `-m 1010`, programs that speak 736 |

A **pin-change interrupt** on A1 switches immediately and **drops any half-finished 5-byte USB frame** so 847 and 736 bytes are never mixed. Frequency / PTT / sat flags in RAM stay put — flip the switch mid-QSO without sending CAT OFF.

### CAT override (three-way, user opcode)

Jumper is default. A non-Yaesu block can **temporarily** ignore A1:

```
  A5 73 36  <sel>  FC
```

| `<sel>` | Source | Effective dialect |
| --- | --- | --- |
| `00` | **Jumper** | Follow A1 again (IRQ live) |
| `01` | Force **847** | 847 even if A1 is low |
| `02` | Force **736** | 736 even if A1 is high |
| `FF` | **Cycle** | jumper → 847 → 736 → jumper |

Reply is the same 32-byte dump as `… FE` (byte 3 = proto in the low nibble, source in the high nibble). Nothing is sent to the radio.

While forced 847/736, A1 IRQs still **latch** the pin but do **not** change the dialect until:

- CAT `sel=00` / cycle back to jumper, or
- **Double-flip** the jumper: two **debounced** edges within **1 s** (wiggle 1→0→1 or 0→1→0) returns source to jumper and follows the pin.

Bounce: PCINT only marks “dirty”. The pin is sampled after **50 ms** of quiet. A single noisy flip is one qualified edge, not a revert. Two stable flips inside a second cancel a CAT force.

This opcode is accepted in **both** dialects so you can switch without already matching the jumper.

---

## If you speak 736 to an 847-mode adapter (A1 HIGH)

It will **not** NAK. Unknown opcodes are **dropped with no USB error**. Overlap is large enough that simplex can look fine while satellite is silently dead.

### Same opcode — usually works

| 736 command | What happens in 847 mode |
| --- | --- |
| CAT ON `00` | Works, **and** forces 145.000 FM onto the radio (847 CAT-ON extra) |
| CAT OFF `80` | Works (only byte 4 matters) |
| Set main freq `01` | Works |
| Set mode `07` | Works |
| PTT `08` / `88` | Works |
| Repeater `09` / `49` / `89`, offset `F9` | Works |
| Mode sat RX/TX `17` / `27` | Forwarded, but duplex may never have been turned on |

### Native 736 opcodes — **locked out** in 847 mode

| 736 opcode | 847 opcode we expect | Result if you send the 736 form |
| --- | --- | --- |
| Full duplex ON **`0E`** | `4E` (we emit `0E`) | Dropped. Sat never enables |
| Sat RX freq **`1E`** | `11` → `1E` | Dropped |
| Sat TX freq **`2E`** | `21` → `2E` | Dropped |
| Tone **`FA`** | `0B` → `FA` | Dropped |
| CTCSS as opcode `4A`/`8A` | `0A` + param in byte 0 | Dropped |

That is why SatPC32/Hamlib **must** be set to **FT-847** when A1 is high. A 736 driver will move VFO A and PTT and believe duplex is on.

### Reads in 847 mode

`E7`/`F7` are answered from the **dummy 847 cache** (5 bytes). Hamlib 736 would not time out, but DCD/S-meter meaning is wrong. Hamlib 736 does not send `03`/`13`/`23`.

Sharp edge: 736 CTCSS-on `{00,00,00,00,0A}` in 847 mode copies byte 0 into the opcode → **CAT ON** again (front panel re-locks). In 736 mode that same block is passed through as TSQL.

---

## If you speak 847 to a 736-mode adapter (A1 LOW)

847-only opcodes are **locked out**: `4E`, `11`, `21`, `0B`. Sat from HRD/SatPC32 will not work until A1 is high again.

Native 736 sat **does** work: `0E`, `1E`, `2E`, `FA`, `4A`/`8A`/`0A` as opcodes. `E7`/`F7` are sent to the radio and the 5-byte reply is returned on USB (real meter/squelch). CAT ON is a single `00` (no forced 145/FM). Duplex OFF `8E` does not rewrite main VFO.

The state machine still tracks freq/mode/PTT/sat from whatever **did** get through, so you can switch A1 back to 847 and the caches remain.

---

## Opcode map (both modes)

| Opcode | 847 mode (A1 HIGH) | 736 mode (A1 LOW) |
| --- | --- | --- |
| `00` / `80` | CAT on (plus sync freq+FM) / off | CAT on (bare) / off |
| `01` | set main, 23 cm `+0xA0` | pass through; cache 23 cm as 240 MHz |
| `11` / `21` | → `1E` / `2E`, sat freq | **locked** |
| `1E` / `2E` | **locked** | sat RX/TX freq, `sat=1` |
| `07` `17` `27` | set mode | same |
| `08` `88` | PTT | same |
| `4E` | → `0E`, sat on | **locked** |
| `0E` | **locked** | full duplex / sat on |
| `8E` | sat off + restore main `01` | sat off only |
| `0B` | tone → `FA` | **locked** |
| `FA` `4A` | **locked** | pass through |
| `0A` | 847 CTCSS param in byte 0 | pass through as 736 TSQL opcode |
| `E7` `F7` | dummy 847 cache to USB | query radio, 5-byte reply to USB |
| `03` `13` `23` | cache to USB | cache to USB (736 software rarely sends these) |
| `A5 73 36 00 FE` | 32-byte state dump (both modes) | same |

---

## Hardware

```
  A1 --o  INPUT_PULLUP
       |
       +-- optional SPDT or jumper to GND = 736 native

  Open / HIGH = 847 (Hamlib 1001)
  GND  / LOW  = 736 (Hamlib 1010)
```

PCINT on A1 (not INT0/INT1 — those pins are PTT MOSFET and S-meter PWM). Bounce may fire the ISR several times; each edge samples the pin and sets the matching protocol.

Do **not** change PC software baud: both radios are 4800 8N2. Only the opcode dialect changes.
