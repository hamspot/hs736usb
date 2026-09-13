# Archives — how HS-736USB got here

This folder holds the pre-Ham-Spot material: Dave KA6BFB’s Windows
proof of concept, Chuck N6BIL’s PIC hardware, and the Comcast pages
that used to host both. The live product tree is still at the repo
root (`Firmware/`, `Hardware/`, `Arduino/`, `Manual/`).

## Narrative

The Yaesu **FT-736R** is still a serious VHF/UHF satellite radio. Its
CAT jack is TTL serial at 4800 8N2, write-mostly, and it locks the
front panel while CAT is on. Ham Radio Deluxe never learned that
dialect. It *did* learn the later **FT-847**.

In **2010**, Dave Dowler **KA6BFB** built a Windows program that sat
between HRD and the radio: HRD thought it was talking to an 847; the
program translated and drove the 736. It needed virtual serial ports
(VSPE or three real COMs) plus a TTL level shifter. He posted it with
**no support**, a setup video, and the installer. That page and zip
are here as [`Software.md`](Software.md).

Chuck **N6BIL** took the same mapping into a **PIC18F14K50** USB box
so the PC saw a CDC serial port and the radio saw native 736 CAT — no
serial-port chaining. He published Eagle files, a HEX, source (GPL),
a Mouser/Digi-Key BOM, and construction videos. That page and the
original downloads are [`Hardware.md`](Hardware.md).

Ham Spot Inc (David L Norris, KG9AE) put N6BIL’s translator in a
Hammond cabinet, wrote a booklet, and sold a short run so the W9CRC
club station — and anyone else stuck with HRD and a 736 — could talk
to the radio. The PIC firmware, Eagle CAD, and that booklet remain in
this repo.

In **2014** KA6BFB wrote that they had given the idea “to all the
world to see and use,” asked only for credit (even fine print), and
after seeing the Ham Spot manual said the description there was
“more than adequate.” He linked the Ham Spot product from his own
page. There is no separate mail from N6BIL in this Gmail account;
his source was already GPL and published.

**v2.0.0** (2026) ports the translator to an Arduino Nano/Uno,
keeps the PIC tree, and adds a 16×2 LCD, dialect switch, PTT/band
GPIO, and a Nano carrier board. The Yaesu-style operating manual
covers both generations.

## Videos (posted on X, 13 Sep 2026)

Thread: [HamSpot HS-736USB / Arduino port](https://x.com/DavidLNorris/status/2098997812826390888)
(@DavidLNorris).

| Who | What | YouTube | X |
| --- | --- | --- | --- |
| KA6BFB | Windows 847→736 translator in use | [watch](https://www.youtube.com/watch?v=7H98FBQZJ_A) | [post](https://x.com/DavidLNorris/status/2099173398702072277) |
| N6BIL | PIC USB hardware (no serial chaining) | [watch](https://www.youtube.com/watch?v=G8I17jarXMw) | [post](https://x.com/DavidLNorris/status/2099173910470087070) |

Chuck’s construction channel (from the 2011 hardware page):
<http://www.youtube.com/user/zdz801>

## Pages in this folder

| File | What it is |
| --- | --- |
| [`Software.md`](Software.md) | KA6BFB Windows emulator (Wayback 11 Jun 2015) |
| [`Hardware.md`](Hardware.md) | N6BIL PIC hardware emulator (Wayback 25 Dec 2011) |
| [`736 Windows Emulator.ZIP`](736%20Windows%20Emulator.ZIP) | KA6BFB installer |
| [`Hex File.zip`](Hex%20File.zip) | N6BIL HEX (`Project 4.hex`) |
| [`Source code package.zip`](Source%20code%20package.zip) | N6BIL PIC sources |
| [`PCB Files.zip`](PCB%20Files.zip) | N6BIL Eagle rev1 |
| [`HRDto736_Driver.zip`](HRDto736_Driver.zip) | Microchip CDC INF + `usbser.sys` |
| [`736R_HRD tester info sheet.doc`](736R_HRD%20tester%20info%20sheet.doc) | N6BIL driver notes |

Unpacked / later copies in the tree:

- PIC HEX and source: [`Firmware/`](../Firmware/)
- Ham Spot Eagle: [`Hardware/Eagle/`](../Hardware/Eagle/)
- Arduino port: [`Arduino/`](../Arduino/)
- Operating manuals: [`Manual/`](../Manual/)
- Nano carrier: [`Hardware/tscircuit/`](../Hardware/tscircuit/)
