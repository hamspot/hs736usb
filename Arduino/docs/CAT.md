# CAT opcode map

Default host dialect is FT-847 (Hamlib `-m 1001`). Pin **A1** low switches the USB dialect to native FT-736R (`-m 1010`). The radio is always driven with 736 CAT. Full tables and “what if I use the wrong dialect”: `docs/PROTOCOLS.md`.

Physical CAT jack: `docs/CONNECTIONS.md` and `docs/WIRING.md` (AMSAT pin numbers; some manuals are wrong).

See `PLAN.md` for the full opcode table, 23 cm mapping, status-byte layout, and the **Bugs in the PIC firmware** section.

Quick reference:

| Host (847) | Radio (736) |
| --- | --- |
| `00` CAT on | `00` then cached `01` freq and `07` FM |
| `80` CAT off | `80` |
| `08` / `88` PTT | same |
| `01` set freq | `01` (23 cm: first BCD `+ 0xA0` if 240–300 MHz) |
| `11` / `21` sat freq | `1E` / `2E` |
| `03` / `13` / `23` get freq | cache only (5 bytes) |
| `07` / `17` / `27` mode | same opcodes |
| `4E` sat on | `0E` full duplex on |
| `8E` sat off | `8E` then main `01` |
| `09` shift (HRD) | opcode = byte 0 |
| `49` / `89` shift (Hamlib) | forward |
| `0A` CTCSS mode | see PLAN.md; DCS dropped |
| `0B` tone | `FA` + PIC `Tone[]` |
| `E7` / `F7` status | 847: `E7` is last polled 736 S-meter (`F7`) as 847 RX status; `F7` is TX/PTT cache. 736 mode: forwarded to the radio |
| `F9` offset | forward |

Radio bytes are spaced **50 ms** apart (FT-736 manual: 50–200 ms per byte).

User opcode `A5 73 36 <sel> FC`: dialect source `00` jumper, `01` force 847, `02` force 736, `FF` cycle. See `docs/PROTOCOLS.md`.
