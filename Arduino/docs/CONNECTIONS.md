# FT-736R connectors

Pinouts for **every** front- and rear-panel connector on the Yaesu FT-736R.

**Do not trust a random photocopy of the operating manual for CAT or DATA.** Some printings number DIN pins with Japanese/Yaesu order instead of IEC 60130, and some print the DATA jack tip/ring backwards. Sources used here are listed at the end of each section. Where sources disagree, both are shown and the one to wire is marked **use this**.

This file is about the **radio**. Arduino adapter wiring is in `docs/WIRING.md`.

---

## Manual errors (read this first)

| What is wrong | What is true | Source |
| --- | --- | --- |
| CAT pin numbers in some FT-736R manuals and an old Yaesu application note | Pin 1 GND, 2 serial **in** (to radio), 3 busy, 4 serial **out** (from radio), 5 NC, 6 +13.8 V. Pins are **not** numbered sequentially around the shell. | AMSAT W6SHP note |
| European DIN numbering vs Yaesu drawing | Same signals; the **printed pin numbers** on a European DIN drawing may not match the Yaesu drawing. Identify pins by the **keyway + AMSAT map** below, not by a number printed in a manual. | French OM translation note |
| DATA IN/OUT tip/ring reversed on some manuals | **Tip = received data (from radio). Ring = transmit data (to radio). Sleeve = GND.** | AMSAT W6SHP note; English OM text (when it matches) |
| CAT “S.IN / S.OUT” named from the TNC’s point of view | On the **radio**: S.IN is data **into** the 736, S.OUT is data **out of** the 736. Adapter TX goes to S.IN. | HS-736USB pads; AMSAT |
| 736 simplex CAT opcode `88` in some 736 charts | `88` is **PTT off**. Simplex is `89`. | Hamlib `ft736.c` |
| MH-31B8 “just works” in the 8-pin jack | Stock mic is **MH-1B8**. An MH-31B8 needs an **internal** rewire (Yaesu service note); the 8-pin shell is the same family, the mic PCB is not. | mods.dk / Yaesu MH-31 note |

---

## Rear panel map (left to right, typical)

Viewed from behind the radio. Optional antenna jacks exist only if the matching band module is installed.

```
  [ DC pigtail from internal PSU ]     [ 13.8 V DC inlet (white) ]
  [ GND screw ]
  [ CAT  6-pin DIN 240° ]
  [ STBY 5-pin DIN 180° ]
  [ KEY  6.35 mm TRS ]
  [ EXT SPKR  3.5 mm TS ]
  [ PTT  RCA ]
  [ DATA IN/OUT  3.5 mm TRS ]
  [ FUSE ]
  [ AC 3-pin ]
  [ ANT 430  type N ]     [ ANT 144  SO-239 ]
  [ ANT opt lower-left    type N or SO-239 ]   [ ANT opt lower-right  SO-239 ]
```

Lower-left optional jack is 1.2 GHz (type N) **or** 50/220 (SO-239). Lower-right optional is 50 or 220 (SO-239). The 1.2 GHz module fits only the lower-left compartment (rear view).

---

## CAT — 6-pin DIN 240° (270° key)

TTL, 4800 bit/s, 8 data, no parity, **2 stop bits**. This is the jack HS-736 uses.

### Pin map — **use this**

| Pin | Radio name | Direction | Notes |
| --- | --- | --- | --- |
| 1 | GND | — | Adapter GND |
| 2 | S.IN / serial data in | **into radio** | Arduino **D9 TX** (idle high TTL) |
| 3 | Busy | from radio | Unused by this adapter |
| 4 | S.OUT / serial data out | **out of radio** | Arduino **D8 RX** |
| 5 | NC | — | No connect |
| 6 | +13.8 V | from radio | **Do not** feed the Arduino 5 V pin |

### Diagram — rear of **male** plug (pins toward you, cable away)

AMSAT: pins are **not** numbered sequentially. Keyway orientation as they drew it:

```
            rear of male 6-pin DIN plug

                 3           1
                       6
                 5           4
                       2

            1 GND              4 S.OUT  (radio TX, to adapter RX)
            2 S.IN  (radio RX, from adapter TX)
            3 BUSY             5 NC
            6 +13.8 V          (do not use for Arduino power)
```

Looking **into the radio’s CAT socket** (holes), the pattern is mirrored left-right.

### Arduino (this project)

```
  PC USB CDC 4800 8N2
       |
  ATmega328 USART0
       |
  D9 TX  ---------------->  CAT pin 2  S.IN
  D8 RX  <----------------  CAT pin 4  S.OUT
  GND    -----------------  CAT pin 1  GND
  (no wire)                 CAT pin 6  +13.8 V
```

---

## STBY — 5-pin DIN 180° (amplifier ground-on-TX)

Open-collector / relay **ground when that band is transmitting**. Connect pin 1 to the amplifier chassis. Do not put 13.8 V or RF on these pins.

| Pin | Function |
| --- | --- |
| 1 | GND |
| 2 | STBY 430 MHz (ground on TX) |
| 3 | STBY for the module on **J5011** (50 / 220 / 1200 MHz, depending what is fitted) |
| 4 | STBY 144 MHz |
| 5 | STBY for the module on **J5010** (50 / 220 MHz) |

Operating manual table (same numbers):

| Amplifier band | STBY pin |
| --- | --- |
| 430 MHz | 2 |
| Band module at J5011 | 3 |
| 144 MHz | 4 |
| Band module at J5010 | 5 |

### Diagram — 5-pin DIN 180°, looking **into the radio socket**

```
            1                 2
                 4       5
                      3
```

(IEC 60130-9 180° layout. If your plug’s key does not match, go by continuity to chassis on pin 1, not by a photocopied number.)

---

## KEY — 6.35 mm TRS (stereo)

| Contact | Function |
| --- | --- |
| Tip | Dash / straight-key | Closed to sleeve keys the TX (or dash if Keyer Unit B is on) |
| Ring | Dot (paddle) | Dual-lever paddle only |
| Sleeve | GND | |

Open-circuit about **+4.5 V**, closed **~2 mA**.

```
        6.35 mm TRS plug
    =========()===========
     tip   ring    sleeve
      |      |        |
    DASH    DOT      GND
    (or straight key ---+--- sleeve)
```

**Never use a mono 2-conductor plug.** A mono barrel shorts ring to sleeve and can hang the optional iambic keyer or the KEY line. For a straight key, use a **stereo** plug and leave the ring unconnected.

If Keyer Unit B is **off** (or not installed), a straight key on tip–sleeve is enough.

---

## EXT SPKR — 3.5 mm TS (mono)

| Contact | Function |
| --- | --- |
| Tip | Speaker audio (follows AF gain) |
| Sleeve | GND |

4–8 Ω (SP-767 or similar). Inserting a plug does **not** mute the front PHONES jack; **PHONES** mutes the internal / this speaker.

```
     3.5 mm TS
    =====()====
     tip  sleeve
      |     |
     AF    GND
```

---

## PTT — RCA (phono)

Parallel with the front **MOX** button. Ground the center pin to transmit.

| Contact | Function |
| --- | --- |
| Center | PTT (active low) |
| Shell | GND |

Open **8 V**, closed **8 mA**. Packet TNC or footswitch. MOX and the mic PTT close this same line.

The HS-736 adapter can **sense** this jack on **A0** (Y-cable) and pass it through to the D2 MOSFET. **8 V will kill a 328 pin** — use the 22 kΩ + 5.1 V zener in `docs/WIRING.md`. CAT PTT still keys the radio over serial; the MOSFET follows CAT **or** this sense line.

```
      RCA
     ( o )   center = PTT
     (===)   shell  = GND
```

---

## DATA IN/OUT — 3.5 mm TRS (packet / AFSK)

Direct FM modulator / discriminator. **No** pre- or de-emphasis. FM mode only at this jack. Good for 1200 baud AFSK; 9600 baud needs internal taps (G3RUH / W6SHP).

### Pin map — **use this** (AMSAT + English OM text)

| Contact | Function | Level |
| --- | --- | --- |
| **Tip** | DATA **OUT** — received audio **from the radio** (RD) | ≤ 200 mVrms, 10 kΩ |
| **Ring** | DATA **IN** — AFSK **to the radio** (TD) | 30 mVrms, 600 Ω |
| **Sleeve** | GND | |

```
        3.5 mm TRS
    =========()===========
     tip   ring    sleeve
      |      |        |
    RD     TD        GND
   (from   (to
    radio)  radio)
```

Some manuals swap tip and ring in the **drawing** even when the text is correct. If a TNC wiring “from the book” has no RX audio, swap tip and ring **once** before changing anything else.

PTT for packet is the **RCA**, not this jack.

---

## 13.8 V DC

| Connector | Role |
| --- | --- |
| White 13.8 V **inlet** | External DC (E-736 cable) **or** the pigtail from the internal switcher |
| Pigtail from internal PSU | 13.8 V at up to 8 A. Plug it into the white inlet for AC-mains use |
| GND screw | Station earth — short, fat, to a real ground |

Do not power the Arduino from CAT pin 6. Do not back-feed the internal PSU.

---

## AC mains and fuse

3-pin AC inlet. Fuse rating must match the **voltage label** on the rear (100 / 117 / 220 / 234 V versions exist). Confirm the label before plugging in.

---

## Antenna jacks

All 50 Ω unbalanced.

| Jack (rear view) | Connector | Band |
| --- | --- | --- |
| Upper left | Type **N** | 430 MHz (always present) |
| Upper right | **SO-239** (type M) | 144 MHz (always present) |
| Lower left (optional) | Type **N** if 1.2 GHz module; **SO-239** if 50 or 220 | Module in that bay only |
| Lower right (optional) | **SO-239** | 50 or 220 MHz module |

```
   rear of radio (typical)

      [ N 430 ]     [ SO-239 144 ]
      [ opt N/SO ]  [ opt SO-239 ]
```

1.2 GHz module: lower-left bay only.

---

## Front panel connectors

### MIC — 8-pin locking round (male on the mic)

Stock mic: **MH-1B8**. Desk: **MD-1B8**. Impedance 200 Ω–10 kΩ, 600 Ω nominal.

Yaesu 8-pin family used on FT-736 / FT-726 / FT-747 / FT-757 / FT-847 (W9DUP compilation). **Pin 2 is earth on the 736**, not +5 V (later HF radios put +5 V on pin 2).

| Pin | FT-736 / MH-1B8 |
| --- | --- |
| 1 | UP |
| 2 | Earth (UP/DOWN common on some mics in this family) |
| 3 | DOWN |
| 4 | FAST scan |
| 5 | Earth |
| 6 | PTT (ground to TX) |
| 7 | Mic screen / earth |
| 8 | Mic audio |

### Diagram — looking **into the radio MIC jack** (holes)

Standard Yaesu 8-pin numbering (pin 8 and 7 at the top):

```
              8           7
          6                   5
          4                   3
              2           1

          8 MIC audio         7 mic GND
          6 PTT               5 GND
          4 FAST              3 DOWN
          2 earth             1 UP
```

An **MH-31B8** is the same 8-pin shell as later radios but **does not** work on a 736 until the mic’s internal PCB is modified (Yaesu). Do not assume pin-for-pin with an unmodified MH-31.

### PHONES — 6.35 mm TRS or TS

| Contact | Function |
| --- | --- |
| Tip | Audio (left, or mono) |
| Ring | Audio (right); on a TS mono plug the ring is unused |
| Sleeve | GND |

4–100 Ω. Stereo or mono headphones. **Inserting a plug cuts the internal speaker and EXT SPKR.**

```
        6.35 mm
    =========()===========
     tip   ring    sleeve
     L     R        GND
```

---

## Front / top controls that are not connectors

Listed so they are not confused with jacks:

| Location | What it is |
| --- | --- |
| POWER, MOX | Front pushbuttons. MOX is in parallel with rear PTT RCA |
| MIC / DRIVE, SQL / TONE, AF / RF | Concentric pots |
| KEYER, VOX GAIN, DELAY | Keyer Unit B and VOX; KEYER must be **out** if using a straight key |
| SAT, AGC | Satellite / AGC. CAT needs VFO + sat VFO, not memory |

---

## HS-736 adapter vs other rear jacks

This firmware only talks to **CAT**. It does not use STBY, KEY, DATA, or MIC.

| You want | Use |
| --- | --- |
| Computer CAT (this project) | CAT DIN, TTL, 4800 8N2 |
| Packet 1200 AFSK | DATA TRS + PTT RCA |
| Packet 9600 | Internal discriminator / varactor taps, not the DATA jack as-is |
| Linear amplifier T/R | STBY DIN, band-specific pin |
| Footswitch / TNC PTT | PTT RCA (or MIC pin 6) |
| CW key | KEY TRS, stereo plug |

---

## Sources

- AMSAT W6SHP, “Note for FT-736 Microsat Mods” — CAT pin correction; DATA tip/ring correction; CAT male-plug map.
- Yaesu FT-736R operating manual (rear-panel chapter) — STBY table, KEY warning, PTT voltages, DATA levels, antenna types, PHONES, MIC impedance. **CAT numbers in some copies are wrong; DATA drawings in some copies are wrong.**
- French OM translation — DIN numbering vs European drawings; STBY pin list; KEY 4.5 V / 2 mA; never use a mono KEY plug.
- Hamlib `rigs/yaesu/ft736.c` — CAT 4800 8N2; simplex opcode `89` not `88`.
- W9DUP *Microphone connectors* compilation — 8-pin map grouping FT-736 with FT-747 / FT-757 / FT-847 (pin 2 earth on 736).
- mods.dk “Modifying the MH-31 for the FT-736R” — MH-31B8 is not drop-in.
- hamspot/hs736usb hardware — TTL S-IN / S-OUT pads, no MAX232.
