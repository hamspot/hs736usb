# Manuals

Two booklets live here.

| File | What it is |
| --- | --- |
| [`HS-736USB-Manual.pdf`](HS-736USB-Manual.pdf) (and `.doc`) | Original 2014 Ham Spot PIC cabinet booklet (HRD, Windows CDC driver). |
| [`HS-736USB-Operating-Manual.pdf`](HS-736USB-Operating-Manual.pdf) | Yaesu-style operating manual for the CAT box and basic FT-736R use. Covers the PIC cabinet and the Arduino / ATmega328P port. |

Rebuild the operating manual (needs `python3-reportlab`):

```
python3 Manual/build_om.py
```

Figures are in `figures/`. Photographs of the FT-736R (Figs. 1, 3–7) are short excerpts from the Yaesu FT-736R Operating Manual, used here under fair use for identification and instruction. They remain copyright Yaesu Musen Co., Ltd. This is not a Yaesu publication. Fig. 9 is from the original Ham Spot booklet. Figs. 2 and 8 are PIC cabinet artwork. Figs. 2A and 10 are the Arduino Nano carrier (`Hardware/tscircuit/`).

Opcode-level notes: [`Arduino/docs/`](../Arduino/docs/).
