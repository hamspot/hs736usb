#!/usr/bin/env python3
"""Build the HS-736USB operating manual (Yaesu OM visual style).

Run from anywhere:
    python3 Manual/build_om.py

Output:
    Manual/HS-736USB-Operating-Manual.pdf

FT-736R photographs are short excerpts from the Yaesu FT-736R Operating
Manual, used here for identification and instruction. They remain
copyright Yaesu Musen Co., Ltd. This file is not a Yaesu publication.
"""

from __future__ import annotations

import os
from reportlab.lib import colors
from reportlab.lib.enums import TA_CENTER, TA_JUSTIFY, TA_LEFT, TA_RIGHT
from reportlab.lib.pagesizes import letter
from reportlab.lib.styles import ParagraphStyle, getSampleStyleSheet
from reportlab.lib.units import inch
from reportlab.platypus import (
    BaseDocTemplate,
    Frame,
    HRFlowable,
    Image,
    KeepTogether,
    NextPageTemplate,
    PageBreak,
    PageTemplate,
    Paragraph,
    Spacer,
    Table,
    TableStyle,
)
from reportlab.pdfgen.canvas import Canvas
from reportlab.lib.utils import ImageReader

HERE = os.path.dirname(os.path.abspath(__file__))
FIG = os.path.join(HERE, "figures")
OUT = os.path.join(HERE, "HS-736USB-Operating-Manual.pdf")

PAGE_W, PAGE_H = letter
MARGIN_L = 0.72 * inch
MARGIN_R = 0.72 * inch
MARGIN_T = 0.78 * inch
MARGIN_B = 0.72 * inch
COL_W = PAGE_W - MARGIN_L - MARGIN_R

GREY_BAR = colors.Color(0.42, 0.42, 0.44)
NEAR_BLACK = colors.Color(0.08, 0.08, 0.08)
RULE = colors.Color(0.12, 0.12, 0.12)
WARN_BG = colors.Color(0.96, 0.96, 0.96)
LIGHT = colors.Color(0.93, 0.93, 0.93)
HEAD_GREY = colors.Color(0.35, 0.35, 0.37)


def fig_path(name: str) -> str:
    p = os.path.join(FIG, name)
    if not os.path.isfile(p):
        raise FileNotFoundError(p)
    return p


def sized_image(name: str, max_w: float, max_h: float | None = None) -> Image:
    path = fig_path(name)
    ir = ImageReader(path)
    iw, ih = ir.getSize()
    aspect = ih / float(iw)
    w = max_w
    h = w * aspect
    if max_h is not None and h > max_h:
        h = max_h
        w = h / aspect
    img = Image(path, width=w, height=h)
    img.hAlign = "CENTER"
    return img


def make_styles():
    ss = getSampleStyleSheet()
    styles = {
        "body": ParagraphStyle(
            "OMBody",
            parent=ss["Normal"],
            fontName="Helvetica",
            fontSize=9.5,
            leading=12.4,
            alignment=TA_JUSTIFY,
            textColor=NEAR_BLACK,
            spaceAfter=8,
        ),
        "bodyleft": ParagraphStyle(
            "OMBodyLeft",
            parent=ss["Normal"],
            fontName="Helvetica",
            fontSize=9.5,
            leading=12.4,
            alignment=TA_LEFT,
            textColor=NEAR_BLACK,
            spaceAfter=8,
        ),
        "section": ParagraphStyle(
            "OMSection",
            parent=ss["Normal"],
            fontName="Helvetica-Bold",
            fontSize=11,
            leading=14,
            textColor=NEAR_BLACK,
            spaceBefore=4,
            spaceAfter=4,
            keepWithNext=True,
        ),
        "sub": ParagraphStyle(
            "OMSub",
            parent=ss["Normal"],
            fontName="Helvetica-Bold",
            fontSize=10,
            leading=13,
            textColor=NEAR_BLACK,
            spaceBefore=10,
            spaceAfter=5,
            keepWithNext=True,
        ),
        "item": ParagraphStyle(
            "OMItem",
            parent=ss["Normal"],
            fontName="Helvetica-Bold",
            fontSize=9.5,
            leading=12,
            textColor=NEAR_BLACK,
            spaceBefore=6,
            spaceAfter=2,
        ),
        "caption": ParagraphStyle(
            "OMCaption",
            parent=ss["Normal"],
            fontName="Helvetica-Oblique",
            fontSize=8,
            leading=10.2,
            alignment=TA_CENTER,
            textColor=HEAD_GREY,
            spaceBefore=3,
            spaceAfter=10,
        ),
        "toc": ParagraphStyle(
            "OMToc",
            parent=ss["Normal"],
            fontName="Helvetica",
            fontSize=10,
            leading=16,
            textColor=NEAR_BLACK,
        ),
        "cell": ParagraphStyle(
            "OMCell",
            parent=ss["Normal"],
            fontName="Helvetica",
            fontSize=8.2,
            leading=10.6,
            textColor=NEAR_BLACK,
        ),
        "cellb": ParagraphStyle(
            "OMCellB",
            parent=ss["Normal"],
            fontName="Helvetica-Bold",
            fontSize=8.2,
            leading=10.6,
            textColor=NEAR_BLACK,
        ),
        "warn": ParagraphStyle(
            "OMWarn",
            parent=ss["Normal"],
            fontName="Helvetica-Bold",
            fontSize=8.4,
            leading=11.2,
            alignment=TA_CENTER,
            textColor=NEAR_BLACK,
        ),
        "small": ParagraphStyle(
            "OMSmall",
            parent=ss["Normal"],
            fontName="Helvetica",
            fontSize=8.2,
            leading=10.8,
            alignment=TA_JUSTIFY,
            textColor=NEAR_BLACK,
            spaceAfter=6,
        ),
        "center": ParagraphStyle(
            "OMCenter",
            parent=ss["Normal"],
            fontName="Helvetica",
            fontSize=9.5,
            leading=12.4,
            alignment=TA_CENTER,
            textColor=NEAR_BLACK,
            spaceAfter=8,
        ),
        "coversub": ParagraphStyle(
            "OMCoverSub",
            parent=ss["Normal"],
            fontName="Helvetica",
            fontSize=11,
            leading=14,
            alignment=TA_CENTER,
            textColor=NEAR_BLACK,
        ),
    }
    return styles


S = make_styles()


def P(text: str, style: str = "body") -> Paragraph:
    return Paragraph(text, S[style])


def section(title: str):
    return KeepTogether(
        [
            P(title, "section"),
            HRFlowable(
                width="100%",
                thickness=1.15,
                color=RULE,
                spaceBefore=0,
                spaceAfter=8,
            ),
        ]
    )


def caption(text: str) -> Paragraph:
    return P(text, "caption")


def figure(name: str, cap: str, max_w: float, max_h: float | None = None):
    return KeepTogether([sized_image(name, max_w, max_h), caption(cap)])


def caution(body: str, title: str = "CAUTION"):
    head = Paragraph(f"- ! ! ! ! ! !  {title}  ! ! ! ! ! ! -", S["warn"])
    txt = Paragraph(body, S["warn"])
    inner = Table(
        [[head], [Spacer(1, 4)], [txt]],
        colWidths=[COL_W - 18],
    )
    inner.setStyle(
        TableStyle(
            [
                ("ALIGN", (0, 0), (-1, -1), "CENTER"),
                ("VALIGN", (0, 0), (-1, -1), "MIDDLE"),
                ("LEFTPADDING", (0, 0), (-1, -1), 8),
                ("RIGHTPADDING", (0, 0), (-1, -1), 8),
                ("TOPPADDING", (0, 0), (-1, -1), 2),
                ("BOTTOMPADDING", (0, 0), (-1, -1), 2),
            ]
        )
    )
    box = Table([[inner]], colWidths=[COL_W])
    box.setStyle(
        TableStyle(
            [
                ("BOX", (0, 0), (-1, -1), 1.4, NEAR_BLACK),
                ("INNERGRID", (0, 0), (-1, -1), 0, colors.white),
                ("BACKGROUND", (0, 0), (-1, -1), WARN_BG),
                ("LEFTPADDING", (0, 0), (-1, -1), 8),
                ("RIGHTPADDING", (0, 0), (-1, -1), 8),
                ("TOPPADDING", (0, 0), (-1, -1), 8),
                ("BOTTOMPADDING", (0, 0), (-1, -1), 8),
            ]
        )
    )
    return KeepTogether([Spacer(1, 6), box, Spacer(1, 10)])


def note_box(body: str, title: str = "NOTE"):
    head = Paragraph(f"<b>{title}</b>", S["cellb"])
    txt = Paragraph(body, S["cell"])
    box = Table([[head], [txt]], colWidths=[COL_W])
    box.setStyle(
        TableStyle(
            [
                ("BOX", (0, 0), (-1, -1), 0.7, NEAR_BLACK),
                ("BACKGROUND", (0, 0), (-1, 0), LIGHT),
                ("LEFTPADDING", (0, 0), (-1, -1), 8),
                ("RIGHTPADDING", (0, 0), (-1, -1), 8),
                ("TOPPADDING", (0, 0), (-1, -1), 5),
                ("BOTTOMPADDING", (0, 0), (-1, -1), 5),
                ("VALIGN", (0, 0), (-1, -1), "TOP"),
            ]
        )
    )
    return KeepTogether([Spacer(1, 4), box, Spacer(1, 8)])


def om_table(headers, rows, col_widths):
    cell, cellb = S["cell"], S["cellb"]
    data = [[Paragraph(h, cellb) for h in headers]]
    for row in rows:
        data.append([Paragraph(c, cell) for c in row])
    t = Table(data, colWidths=col_widths, repeatRows=1)
    t.setStyle(
        TableStyle(
            [
                ("BACKGROUND", (0, 0), (-1, 0), LIGHT),
                ("FONTNAME", (0, 0), (-1, 0), "Helvetica-Bold"),
                ("GRID", (0, 0), (-1, -1), 0.4, colors.Color(0.45, 0.45, 0.45)),
                ("VALIGN", (0, 0), (-1, -1), "TOP"),
                ("LEFTPADDING", (0, 0), (-1, -1), 5),
                ("RIGHTPADDING", (0, 0), (-1, -1), 5),
                ("TOPPADDING", (0, 0), (-1, -1), 4),
                ("BOTTOMPADDING", (0, 0), (-1, -1), 4),
            ]
        )
    )
    return t


def draw_cover(c: Canvas, _doc):
    c.saveState()
    bar = 1.58 * inch
    c.setFillColor(GREY_BAR)
    c.rect(0, 0, bar, PAGE_H, fill=1, stroke=0)
    c.setFillColor(colors.white)
    letters = list("HS-736USB")
    top = PAGE_H - 1.15 * inch
    gap = 0.64 * inch
    c.setFont("Helvetica-Bold", 26)
    for i, ch in enumerate(letters):
        c.drawCentredString(bar / 2.0, top - i * gap, ch)

    x = bar + 0.55 * inch
    y = PAGE_H - 1.55 * inch
    c.setFillColor(NEAR_BLACK)
    c.setFont("Helvetica-Bold", 22)
    c.drawString(x, y, "OPERATING")
    c.drawString(x, y - 28, "MANUAL")

    c.setStrokeColor(NEAR_BLACK)
    c.setLineWidth(1.6)
    c.line(x, y - 44, PAGE_W - 0.7 * inch, y - 44)

    c.setFont("Helvetica-Bold", 26)
    c.drawString(x, y - 82, "HS-736USB")
    c.setFont("Helvetica", 13)
    c.drawString(x, y - 104, "FT-736R  CAT  INTERFACE")

    c.setFont("Helvetica", 10)
    c.drawString(x, y - 132, "Arduino / ATmega328P edition")

    # thin rules like Yaesu contents ghost
    c.setLineWidth(0.5)
    c.line(x, 2.55 * inch, PAGE_W - 0.7 * inch, 2.55 * inch)
    c.setFont("Helvetica-Bold", 11)
    c.drawString(x, 2.25 * inch, "HAM SPOT INC")
    c.setFont("Helvetica", 9)
    c.drawString(x, 2.08 * inch, "http://hamspot.com/")
    c.drawString(x, 1.90 * inch, "FT-736R to computer CAT -- USB")
    c.setFont("Helvetica", 8)
    c.setFillColor(HEAD_GREY)
    c.drawString(x, 0.85 * inch, "Not a Yaesu publication.  See notice inside.")
    c.restoreState()


def draw_interior(c: Canvas, doc):
    if getattr(doc, "pageTemplate", None) and doc.pageTemplate.id == "cover":
        return
    c.saveState()
    c.setFillColor(NEAR_BLACK)
    c.setFont("Helvetica", 7.5)
    c.drawString(MARGIN_L, PAGE_H - 0.48 * inch, "HS-736USB  OPERATING MANUAL")
    c.drawRightString(
        PAGE_W - MARGIN_R, PAGE_H - 0.48 * inch, "FT-736R CAT INTERFACE"
    )
    c.setStrokeColor(NEAR_BLACK)
    c.setLineWidth(0.9)
    y_rule = PAGE_H - 0.56 * inch
    c.line(MARGIN_L, y_rule, PAGE_W - MARGIN_R, y_rule)
    c.setLineWidth(0.35)
    c.line(MARGIN_L, y_rule - 2.2, PAGE_W - MARGIN_R, y_rule - 2.2)

    c.setLineWidth(0.9)
    c.line(MARGIN_L, 0.50 * inch, PAGE_W - MARGIN_R, 0.50 * inch)
    c.setLineWidth(0.35)
    c.line(MARGIN_L, 0.50 * inch + 2.2, PAGE_W - MARGIN_R, 0.50 * inch + 2.2)
    c.setFont("Helvetica", 9)
    # Cover is PDF page 1; interior numbering starts at 1.
    c.drawCentredString(PAGE_W / 2.0, 0.32 * inch, f"- {doc.page - 1} -")
    c.restoreState()


def build_story():
    story = []
    w = COL_W

    # --- page 1 after cover: notice + contents ---
    story.append(P("NOTICE", "section"))
    story.append(
        HRFlowable(width="100%", thickness=1.15, color=RULE, spaceBefore=0, spaceAfter=8)
    )
    story.append(
        P(
            "This booklet is the operating manual for the <b>HS-736USB</b> "
            "computer-aided transceiver interface: a small USB box that lets "
            "modern amateur-radio software talk to a Yaesu <b>FT-736R</b> as if "
            "the radio were an <b>FT-847</b>. It also covers the few front- and "
            "rear-panel operations you need on the 736 itself so that CAT "
            "control will work."
        )
    )
    story.append(
        P(
            "It is <b>not</b> a Yaesu publication and does not replace the "
            "FT-736R Operating Manual or Technical Supplement. FT-736R, FT-847 "
            "and YAESU are trademarks of Yaesu Musen Co., Ltd. Photographs of "
            "the transceiver in this booklet are short excerpts from the "
            "original Yaesu FT-736R Operating Manual, reproduced here under "
            "fair use for identification and instruction only. Those images "
            "remain copyright Yaesu Musen."
        )
    )
    story.append(
        P(
            "The HS-736USB firmware (PIC original by N6BIL, Arduino port in "
            "this repository) is licensed under the GNU General Public License "
            "version 3. Hardware CAD, this booklet, and the remainder of the "
            "Ham Spot tree follow the same license unless a file says otherwise."
        )
    )
    story.append(
        P(
            "Credits: Dave KA6BFB (software proof of concept), Chuck N6BIL "
            "(PIC hardware translator, GPL), Ham Spot Inc (HS-736USB product), "
            "and the Arduino / ATmega328P port in <b>Arduino/</b>."
        )
    )

    story.append(P("CONTENTS", "section"))
    story.append(
        HRFlowable(width="100%", thickness=1.15, color=RULE, spaceBefore=0, spaceAfter=8)
    )
    toc = [
        "Section 1.  General Description",
        "Section 2.  The FT-736R at a Glance",
        "Section 3.  Installation of the Interface",
        "Section 4.  Basic Operation of the Radio with CAT",
        "Section 5.  Computer Software",
        "Section 6.  FT-847 and FT-736R Dialects",
        "Section 7.  Accessory GPIO (Arduino / Nano carrier)",
        "Section 8.  In Case of Difficulty",
        "Appendix A.  CAT Pin Map (use this)",
        "Appendix B.  Specifications of the Interface",
    ]
    for line in toc:
        story.append(P(line, "toc"))

    story.append(PageBreak())

    # --- §1 ---
    story.append(section("Section 1.  GENERAL DESCRIPTION"))
    story.append(
        figure(
            "fig-radio-34.png",
            "Fig. 1  Yaesu FT-736R VHF/UHF multimode base (excerpt from the Yaesu Operating Manual).",
            w * 0.88,
            max_h=2.35 * inch,
        )
    )
    story.append(P("1.1  The HS-736USB Interface", "sub"))
    story.append(
        P(
            "The FT-736R remains a first-class satellite and VHF/UHF station. "
            "Its CAT (Computer Aided Transceiver) jack is TTL serial at "
            "<b>4800 bit/s, 8 data bits, no parity, 2 stop bits</b>. Many "
            "current programs \u2014 Ham Radio Deluxe in particular \u2014 never "
            "learned the 736 dialect. They <i>did</i> learn the later FT-847."
        )
    )
    story.append(
        P(
            "The HS-736USB sits between the computer and the radio. On USB it "
            "presents an FT-847 CAT port. On the DIN cable it speaks native "
            "FT-736R CAT, including the 23 cm mapping the 847 never had. "
            "Features the 736 does not have are ignored. Memory channels are "
            "<b>not</b> supported: put the radio on a VFO, and satellite mode "
            "on VFO, before you turn CAT on."
        )
    )
    story.append(
        P(
            "Two hardware generations exist. The original Ham Spot box is a "
            "PIC18F14K50 in a Hammond enclosure (see <b>"
            "Firmware/</b> and <b>Hardware/Eagle/</b>). The "
            "Arduino / ATmega328P port in <b>Arduino/</b> "
            "is a drop-in replacement of that translator. The "
            "<b>Nano carrier</b> in <b>Hardware/tscircuit/</b> "
            "is the board that generation sits on: the Nano plugs in; CAT, "
            "PTT MOSFET, RCA sense, dialect jumper, LCD, band and S-meter "
            "PWM are on the carrier. User operation of CAT is the same on "
            "PIC and Arduino."
        )
    )
    story.append(
        figure(
            "fig-pcb.png",
            "Fig. 2  Original HS-736USB printed circuit (v2 artwork, PIC18F14K50). USB Mini-B to the computer; S-IN / S-OUT / GND to the radio CAT jack.",
            w * 0.70,
            max_h=1.75 * inch,
        )
    )
    story.append(
        figure(
            "fig-nano-pcb.png",
            "Fig. 2A  Arduino Nano carrier (tscircuit). The Nano plugs into JP_L / JP_R (USB at the top). J_CAT is the 6-pin AMSAT CAT jack; pin 6 is not connected. Power is Nano USB 5 V. Red = top copper, blue = bottom.",
            w * 0.62,
            max_h=2.35 * inch,
        )
    )
    story.append(P("1.2  What You Need", "sub"))
    story.append(
        P(
            "\u2022 An FT-736R with CAT enabled in the usual way (see \u00a74).<br/>"
            "\u2022 The HS-736USB (PIC cabinet or Arduino Nano carrier), powered from USB.<br/>"
            "\u2022 A 6-pin DIN 240\u00b0 (270\u00b0 key) cable, wired to the AMSAT "
            "map in Appendix A \u2014 <b>not</b> blindly from a photocopied "
            "Yaesu pin-number drawing.<br/>"
            "\u2022 Station software that can speak Yaesu FT-847 CAT at 4800 8N2 "
            "(Ham Radio Deluxe, Hamlib <b>-m 1001</b>, "
            "SatPC32, WSJT-X, \u2026), <i>or</i> native FT-736 CAT "
            "(<b>-m 1010</b>) if you put the Arduino in "
            "736 dialect."
        )
    )

    story.append(section("Section 2.  THE FT-736R AT A GLANCE"))
    story.append(
        P(
            "This section is a short tour of the controls you will actually "
            "touch when the interface is in the station. It is not a substitute "
            "for Section 2 of the Yaesu Operating Manual."
        )
    )
    story.append(P("2.1  Front Panel", "sub"))
    story.append(
        figure(
            "fig-front.png",
            "Fig. 3  Front panel, numbered as in the Yaesu Operating Manual (excerpt).",
            w,
            max_h=2.05 * inch,
        )
    )
    story.append(P("(1)  POWER", "item"))
    story.append(
        P(
            "Turns the transceiver on and off. Apply AC (or 13.8 V DC) and "
            "leave POWER on before you plug the interface into the computer; "
            "CAT ON from the PC will then lock the front panel."
        )
    )
    story.append(P("(2)  MOX", "item"))
    story.append(
        P(
            "Manual transmit. With CAT running, Ham Radio Deluxe\u2019s on-screen "
            "PTT can drop the radio out of satellite mode \u2014 a long-standing "
            "HRD bug. The reliable workaround is to transmit with <b>MOX</b>, "
            "the microphone PTT, or a footswitch on the rear RCA, not with "
            "the program\u2019s PTT button. See \u00a75.3."
        )
    )
    story.append(P("(3)  MIC jack", "item"))
    story.append(
        P(
            "8-pin locking connector for the stock MH-1B8 (or MD-1B8 desk mic). "
            "Pin 2 is earth on the 736, not +5 V. An unmodified MH-31B8 does "
            "<b>not</b> drop in."
        )
    )
    story.append(P("(11)  Main Tuning Knob  \u00b7  (21)  Display", "item"))
    story.append(
        P(
            "Once CAT is on, the front panel is locked and the computer owns "
            "frequency and mode. Set the radio to a VFO (not a memory channel) "
            "<b>before</b> CAT ON. 1240 MHz is shown to FT-847 software as "
            "240 MHz (the 847 has no 4 MHz digit)."
        )
    )
    story.append(
        figure(
            "fig-sat-knobs.png",
            "Fig. 4  AGC and SAT selectors (excerpt from the Yaesu Operating Manual).",
            2.6 * inch,
            max_h=1.55 * inch,
        )
    )
    story.append(P("(27)  SAT selector", "item"))
    story.append(
        P(
            "Full-duplex satellite operation. <b>OFF</b> is simplex. "
            "<b>RX</b> / <b>TX</b> display and tune one side while the other "
            "is fixed. <b>NOR</b> tracks both VFOs the same way; <b>REV</b> "
            "tracks them in opposite directions (the usual amateur-satellite "
            "inverting transponder). Leave this on a VFO-related SAT position, "
            "not a memory, before enabling CAT. The computer then turns full "
            "duplex on and off with the 847 <b>4E</b> / "
            "<b>8E</b> (translated to 736 "
            "<b>0E</b> / <b>8E</b>)."
        )
    )

    story.append(P("2.2  Rear Panel", "sub"))
    story.append(
        figure(
            "fig-rear.png",
            "Fig. 5  Rear panel jacks (excerpt from the Yaesu Operating Manual). CAT is item (3), the 6-pin DIN above the KEY jack.",
            w,
            max_h=2.35 * inch,
        )
    )
    story.append(
        P(
            "Typical layout, viewed from behind: DC pigtail from the internal "
            "switcher and the white 13.8 V inlet; ground screw; <b>CAT</b> "
            "6-pin DIN 240\u00b0; <b>STBY</b> 5-pin DIN (amplifier ground-on-TX); "
            "KEY; EXT SPKR; PTT RCA; DATA IN/OUT; fuse; AC inlet; antenna "
            "jacks (N on 430, SO-239 on 144; optional modules below)."
        )
    )
    story.append(
        figure(
            "fig-cat-din.png",
            "Fig. 6  CAT DIN as printed in the Yaesu Operating Manual (4800 bit/s, TTL). Use the AMSAT pin map in Appendix A \u2014 some printings number the shell differently.",
            3.35 * inch,
            max_h=2.15 * inch,
        )
    )
    story.append(
        note_box(
            "Do not trust a random photocopy of the operating manual for CAT "
            "or DATA pin numbers. Some printings number DIN pins with a "
            "Japanese / Yaesu order instead of IEC 60130, and some print the "
            "DATA jack tip/ring backwards. Identify CAT pins by the keyway plus "
            "the AMSAT map (Appendix A). On the radio, S.IN is data "
            "<b>into</b> the 736 (adapter TX); S.OUT is data <b>out of</b> "
            "the 736 (adapter RX).",
            "MANUAL ERRORS",
        )
    )

    story.append(
        KeepTogether(
            [
                section("Section 3.  INSTALLATION OF THE INTERFACE"),
                P("3.1  Powering the Radio", "sub"),
                figure(
                    "fig-power-rear.png",
                    "Fig. 7  AC inlet, DC pigtail and 13.8 V socket (excerpt from the Yaesu Operating Manual).",
                    3.15 * inch,
                    max_h=2.15 * inch,
                ),
            ]
        )
    )
    story.append(
        P(
            "Before connecting mains, read the voltage label on the rear "
            "panel (100 / 117 / 220 / 234 V versions exist) and confirm the "
            "fuse matches that range (4 A on the low range, 2 A on the high "
            "range). For AC operation plug the internal-supply pigtail into "
            "the white 13.8 V inlet, then the AC cord. An external 13.8 V "
            "supply must be able to deliver 8 A continuous."
        )
    )
    story.append(
        caution(
            "PERMANENT DAMAGE WILL RESULT IF IMPROPER SUPPLY VOLTAGE IS "
            "APPLIED TO THE TRANSCEIVER. YOUR WARRANTY DOES NOT COVER DAMAGE "
            "CAUSED BY IMPROPER SUPPLY VOLTAGE OR USE OF AN IMPROPER FUSE. "
            "DO NOT CONNECT THE MAINS CORD UNTIL ALL OTHER INTERCONNECTIONS "
            "HAVE BEEN MADE."
        )
    )
    story.append(P("3.2  Connecting the HS-736USB", "sub"))
    story.append(
        P(
            "Power the interface from USB only. Wire a 6-pin DIN plug as "
            "follows (rear of the <b>male</b> plug, pins toward you; numbers "
            "are <b>not</b> sequential around the shell):"
        )
    )
    story.append(
        om_table(
            ["DIN pin", "Radio name", "Interface"],
            [
                ["1", "GND", "GND"],
                ["2", "S.IN  (serial data in)", "TX to the radio (Arduino D9)"],
                ["3", "BUSY", "PIC: no connect. Nano J_CAT: D10"],
                ["4", "S.OUT (serial data out)", "RX from the radio (Arduino D8)"],
                ["5", "NC", "no connect"],
                ["6", "+13.8 V", "<b>do not connect</b>"],
            ],
            [0.85 * inch, 2.35 * inch, w - 3.20 * inch],
        )
    )
    story.append(Spacer(1, 8))
    story.append(
        P(
            "PIC cabinet: the silkscreen already reads S-IN / S-OUT / GND; "
            "those pads go to pins 2, 4 and 1. Nano carrier: plug the 6-pin "
            "DIN into <b>J_CAT</b> (GND, SIN, BUSY, SOUT, NC, NC). Pin 6 is "
            "not connected. Arduino D9 is AltSoftSerial TX, D8 is RX "
            "(Timer 1). Levels are TTL idle-high, same as the original "
            "HS-736USB. Do not insert a MAX232 unless your particular radio "
            "only clocks inverted CAT."
        )
    )
    story.append(
        caution(
            "DO NOT FEED CAT PIN 6 (+13.8 V) INTO THE ARDUINO 5 V PIN OR "
            "THE PIC VDD RAIL. POWER THE INTERFACE FROM USB. PIN 6 IS A "
            "RADIO SUPPLY OUTPUT, NOT A SIGNAL."
        )
    )
    story.append(
        figure(
            "fig-faceplate.png",
            "Fig. 8  Original HS-736USB cabinet legend (USB Mini-B, CAT to the FT-736R).",
            1.15 * inch,
            max_h=1.75 * inch,
        )
    )
    story.append(P("3.3  The Computer Port", "sub"))
    story.append(
        P(
            "<b>Arduino / ATmega328P.</b> The host USB-serial baud <b>must</b> "
            "be 4800 8N2 with no handshake. The original PIC CDC ignored the "
            "host baud; an Uno or Nano cannot. A genuine Uno (ATmega16U2) "
            "enumerates as Arduino USB serial; many Nano clones use a CH340. "
            "Install the matching driver if the operating system does not "
            "already have one. No Microchip <b>mchpcdc.inf</b> "
            "is required."
        )
    )
    story.append(
        P(
            "<b>Original PIC HS-736USB.</b> Windows enumerates the box as "
            "\u201cCDC RS-232 Emulation Demo\u201d. The 2014 booklet walked through "
            "the Found New Hardware Wizard and Microchip\u2019s INF. That procedure "
            "applies only to the PIC cabinet, not to the Arduino port."
        )
    )
    story.append(
        note_box(
            "Host settings in Hamlib, HRD, SatPC32 and WSJT-X: radio model "
            "<b>Yaesu FT-847</b>, <b>4800</b> baud, 8 data bits, no parity, "
            "2 stop bits, no handshake. Default Arduino dialect is FT-847 "
            "(pin A1 open / HIGH). Ground A1, or send the CAT override in "
            "\u00a76, for native FT-736."
        )
    )

    story.append(section("Section 4.  BASIC OPERATION OF THE RADIO WITH CAT"))
    story.append(P("4.1  Before You Enable CAT", "sub"))
    story.append(
        P(
            "1. POWER the radio. Confirm receive audio and that the correct "
            "antenna jack is in use for the band you intend (144 SO-239, "
            "430 type N, optional modules on the lower pair).<br/>"
            "2. Select a <b>VFO</b> with the VFO / MR keys. Do not leave the "
            "radio on a memory channel. The interface does not handle memories.<br/>"
            "3. If you will run satellite, set the SAT selector to a VFO "
            "position (RX, TX, NOR or REV) \u2014 not a duplex memory. "
            "Simplex: SAT to OFF.<br/>"
            "4. Plug in the CAT cable and the USB cable. The activity LED "
            "(Arduino D13, or the PIC cabinet LED) will blink when bytes move."
        )
    )
    story.append(P("4.2  CAT ON and the Front Panel Lock", "sub"))
    story.append(
        P(
            "When the computer sends CAT ON (<b>00</b>), "
            "the 736 locks its front panel. In FT-847 dialect the adapter also "
            "forces 145.000 MHz FM onto the radio as the 847 CAT-ON extra; "
            "your logging program then writes the real frequency. CAT OFF "
            "(<b>80</b>) unlocks the panel. Do not fight "
            "the lock with the D LOCK button \u2014 turn CAT off from the PC "
            "if you need the knobs."
        )
    )
    story.append(P("4.3  Simplex", "sub"))
    story.append(
        P(
            "Set band and mode from the program (or, before CAT ON, from the "
            "MODE buttons: LSB, USB, FM-N, FM, CW-N, CW). Tune from the "
            "program. PTT is CAT <b>08</b> on / "
            "<b>88</b> off, or MOX / mic / rear RCA. "
            "Repeater shift and CTCSS are translated; DCS from 847-style "
            "commands is dropped. The 736 simplex opcode is "
            "<b>89</b> \u2014 <b>88</b> "
            "is PTT off, despite some old 736 charts."
        )
    )
    story.append(P("4.4  Satellite (full duplex)", "sub"))
    story.append(
        P(
            "With the adapter in FT-847 dialect (the usual HRD / SatPC32 / "
            "Hamlib 1001 case), the PC sends 847 satellite opcodes; the box "
            "rewrites them to 736:"
        )
    )
    story.append(
        om_table(
            ["What you want", "PC (FT-847)", "Radio (FT-736R)"],
            [
                ["Full duplex on", "<b>4E</b>", "<b>0E</b>"],
                ["Sat RX frequency", "<b>11</b>", "<b>1E</b>"],
                ["Sat TX frequency", "<b>21</b>", "<b>2E</b>"],
                ["Sat RX / TX mode", "<b>17</b> / <b>27</b>", "same opcodes"],
                ["Full duplex off", "<b>8E</b>", "<b>8E</b> then restore main VFO"],
                ["Tone", "<b>0B</b>", "<b>FA</b>"],
            ],
            [1.7 * inch, 2.15 * inch, w - 3.85 * inch],
        )
    )
    story.append(Spacer(1, 8))
    story.append(
        P(
            "If the PC is speaking native 736 while the adapter is in 847 "
            "mode, those 736 sat opcodes are <b>dropped with no error</b>. "
            "Simplex will still appear to work. That is the usual \u201cSatPC32 "
            "won\u2019t go duplex\u201d failure \u2014 the program is set to FT-736 "
            "instead of FT-847. See \u00a76."
        )
    )
    story.append(P("4.5  23 cm", "sub"))
    story.append(
        P(
            "The FT-847 has no 4 MHz digit, so 1240\u20131299.99999 MHz is shown "
            "to the PC as 240\u2013299.99999 MHz. The adapter adds "
            "<b>0xA0</b> to the first frequency BCD byte "
            "when it writes the radio. Tune 23 cm in the program as 240 MHz, "
            "not 1240 MHz."
        )
    )
    story.append(P("4.6  After the QSO", "sub"))
    story.append(
        P(
            "Send CAT OFF before you unplug USB or power the radio down. If "
            "the panel stays locked, cycle POWER on the 736; that clears CAT "
            "as surely as <b>80</b>."
        )
    )

    story.append(section("Section 5.  COMPUTER SOFTWARE"))
    story.append(P("5.1  Ham Radio Deluxe", "sub"))
    story.append(
        P(
            "On the Connect window, open the <b>Serial Ports</b> tab and note "
            "which COM (or /dev/ttyUSBn, /dev/ttyACM0) the interface received. "
            "Create a new connection: Company <b>Yaesu</b>, Radio "
            "<b>FT-847</b>, that port, speed <b>4800</b>, CTS/DTR/RTS off."
        )
    )
    story.append(
        figure(
            "fig-hrd-connect.png",
            "Fig. 9  Ham Radio Deluxe Connect window \u2014 Yaesu FT-847 at 4,800 (from the original HS-736USB booklet).",
            w * 0.92,
            max_h=2.55 * inch,
        )
    )
    story.append(P("5.2  Hamlib, SatPC32, WSJT-X", "sub"))
    story.append(
        P(
            "Hamlib: <b>rigctl -m 1001 -r PORT -s 4800</b> "
            "for 847 dialect (default). Native 736 is "
            "<b>-m 1010</b> and requires the adapter in "
            "736 dialect (\u00a76). SatPC32 and WSJT-X: pick FT-847, 4800 8N2, "
            "no flow control. Do not raise the radio CAT baud \u2014 the 736 "
            "stays at 4800; the adapter does not change it."
        )
    )
    story.append(P("5.3  Known Issues with HRD", "sub"))
    story.append(
        P(
            "<b>SAT PTT.</b> If you press SAT on the radio, HRD will show SAT. "
            "If you then use HRD\u2019s software PTT, HRD may drop out of SAT "
            "mode while the 736 is still in SAT. Workaround: transmit with "
            "the radio MOX button, the mic PTT, or a footswitch. Do not use "
            "the on-screen PTT while in satellite mode."
        )
    )
    story.append(
        P(
            "<b>Two-band satellite swap.</b> Changing TX/RX bands in SAT via "
            "HRD is awkward on a two-band 736. A three- or four-band radio is "
            "easier. Example: TX on 2 m and RX on 70 cm \u2014 park TX on "
            "6 m / 220 / 1200 briefly, move RX to 2 m, then put TX on 70 cm."
        )
    )

    story.append(section("Section 6.  FT-847 AND FT-736R DIALECTS"))
    story.append(
        P(
            "The Arduino port (not the original PIC) can speak either "
            "language on USB. The radio is always driven as a 736. Pin "
            "<b>A1</b> is an input with pull-up:"
        )
    )
    story.append(
        om_table(
            ["A1", "USB dialect", "Use with"],
            [
                ["HIGH (open)", "FT-847 (default)", "Hamlib 1001, HRD, SatPC32 as FT-847"],
                ["LOW (jumper to GND)", "FT-736R native", "Hamlib 1010, programs that speak 736"],
            ],
            [1.55 * inch, 1.9 * inch, w - 3.45 * inch],
        )
    )
    story.append(Spacer(1, 8))
    story.append(
        P(
            "A pin-change interrupt switches immediately and drops any "
            "half-finished 5-byte USB frame so the two dialects are never "
            "mixed. Frequency, PTT and sat flags in RAM stay put \u2014 you may "
            "flip the jumper mid-QSO without sending CAT OFF. Bounce is "
            "debounced 50 ms."
        )
    )
    story.append(P("6.1  CAT Override", "sub"))
    story.append(
        P(
            "A non-Yaesu 5-byte block can ignore A1 for a while. It is "
            "accepted in both dialects:"
        )
    )
    story.append(P("<b>A5 73 36  &lt;sel&gt;  FC</b>", "center"))
    story.append(
        om_table(
            ["&lt;sel&gt;", "Source", "Effect"],
            [
                ["00", "Jumper", "Follow A1 again"],
                ["01", "Force 847", "847 even if A1 is low"],
                ["02", "Force 736", "736 even if A1 is high"],
                ["FF", "Cycle", "jumper -&gt; 847 -&gt; 736 -&gt; jumper"],
            ],
            [0.9 * inch, 1.3 * inch, w - 2.2 * inch],
        )
    )
    story.append(Spacer(1, 8))
    story.append(
        P(
            "Two stable jumper flips inside one second (wiggle the pin) "
            "cancel a CAT force and return to jumper-follow. Nothing in this "
            "block is sent to the radio."
        )
    )
    story.append(P("6.2  If You Speak the Wrong Dialect", "sub"))
    story.append(
        P(
            "The adapter will not NAK. Unknown opcodes are dropped with no "
            "USB error. Overlap is large enough that simplex can look fine "
            "while satellite is silently dead."
        )
    )
    story.append(
        P(
            "<b>736 software into an 847-mode adapter:</b> duplex ON "
            "<b>0E</b>, sat frequencies "
            "<b>1E</b>/<b>2E</b> "
            "and tone <b>FA</b> never reach the radio. "
            "736 CTCSS-on <b>{00,00,00,00,0A}</b> is "
            "misread as CAT ON and re-locks the panel."
        )
    )
    story.append(
        P(
            "<b>847 software into a 736-mode adapter:</b> "
            "<b>4E</b>, <b>11</b>, "
            "<b>21</b>, <b>0B</b> "
            "are locked out. HRD/SatPC32 satellite will not work until A1 is "
            "high again. Native 736 sat <i>does</i> work in this mode, and "
            "<b>E7</b>/<b>F7</b> "
            "are queried from the radio instead of a dummy 847 cache."
        )
    )

    story.append(section("Section 7.  ACCESSORY GPIO (ARDUINO / NANO CARRIER)"))
    story.append(
        P(
            "These pins are extras on the Uno/Nano port. They are not FT-847 "
            "CAT. On the Nano carrier they are headers next to the socket "
            "(Fig. 2A / Fig. 10). Prefer the radio\u2019s own STBY 5-pin DIN "
            "for band-keyed linears if you can."
        )
    )
    story.append(
        om_table(
            ["Pin", "Dir.", "Function"],
            [
                ["D2", "out", "PTT MOSFET gate. HIGH when CAT PTT or RCA sense is keyed"],
                ["D3", "PWM", "S-meter (0\u2013255), Timer 2"],
                ["D4\u2013D7", "out", "One-hot 50 / 144 / 220 / 430 MHz"],
                ["D10", "in", "CAT BUSY (LOW = squelch open)"],
                ["D11, D12", "out", "HD44780 RS / E"],
                ["A2\u2013A5", "out", "HD44780 D4\u2013D7"],
                ["A0", "in", "Optional PTT sense (active LOW)"],
                ["A1", "in", "Dialect: HIGH = 847, LOW = 736"],
                ["D8 / D9", "UART", "CAT RX / TX (AltSoftSerial)"],
                ["D13", "out", "Dialect LED (HIGH = 847)"],
            ],
            [1.35 * inch, 0.7 * inch, w - 2.05 * inch],
        )
    )
    story.append(Spacer(1, 8))
    story.append(
        P(
            "No 2-bit band outputs. 1240 MHz is shown on the LCD (host CAT "
            "as 240 MHz). In satellite, one-hot drives <b>both</b> RX and TX "
            "among 50/144/220/430."
        )
    )
    story.append(
        om_table(
            ["Carrier jack", "Pins", "Goes to"],
            [
                ["J_CAT", "GND SIN BUSY SOUT NC NC", "FT-736R CAT DIN (pin 6 open)"],
                ["J_AMP", "COIL12 DRAIN GND", "amp PTT MOSFET drain / coil +12"],
                ["J_RCA", "TIP GND", "radio PTT RCA sense (22 k + zener)"],
                ["JP_A1", "A1 GND", "open = 847 dialect; jumper = 736"],
                ["J_BAND", "D4 D5 D6 D7 GND", "one-hot 50 / 144 / 220 / 430"],
                ["J_PWM", "D3 GND", "S-meter PWM"],
                ["J_LCD", "16-pin HD44780", "16\u00d72, 4-bit; RV1 contrast"],
            ],
            [1.15 * inch, 2.05 * inch, w - 3.20 * inch],
        )
    )
    story.append(Spacer(1, 8))
    story.append(
        figure(
            "fig-nano-sch.png",
            "Fig. 10  Nano carrier schematic (same netlist as Fig. 2A). Nano socket, CAT jack, PTT MOSFET + RCA, HD44780, dialect jumper / LED / band / PWM.",
            w * 0.96,
            max_h=2.45 * inch,
        )
    )
    story.append(P("7.1  PTT MOSFET", "sub"))
    story.append(
        P(
            "D2 -- 100 ohm -- MOSFET gate, 10 kohm to ground. Source to "
            "Arduino GND (common with radio GND). Drain to the amplifier PTT "
            "/ relay (other side of the coil to +12 V, diode across the coil). "
            "A 2N7000 is enough for a small relay; use a logic-level power "
            "MOSFET for bigger coils. Output HIGH = TX = ground the amp PTT "
            "line, same sense as the 736 STBY jack. CAT "
            "<b>08</b>/<b>88</b> "
            "<b>or</b> the sense input keys this pin, so mic / MOX / "
            "footswitch keys the amp even when CAT PTT is idle."
        )
    )
    story.append(P("7.2  Rear RCA PTT Sense (optional)", "sub"))
    story.append(
        P(
            "The 736 PTT RCA is 8 V open, ground-to-TX, 8 mA closed. It is "
            "in parallel with MOX and the mic PTT. <b>8 V will destroy a "
            "328 pin.</b> Y-cable the RCA: one leg still to a footswitch if "
            "you use one; the other through 22 kohm to A0, with a 5.1 V "
            "zener to GND (cathode at A0). Firmware: INPUT_PULLUP, TX = pin "
            "LOW. Unplugged = RX."
        )
    )
    story.append(
        caution(
            "DO NOT WIRE THE 8 V PTT RCA DIRECTLY TO A0. USE THE 22 kohm "
            "SERIES RESISTOR AND 5.1 V ZENER. CAT PTT STILL KEYS THE RADIO "
            "OVER SERIAL; THE MOSFET FOLLOWS CAT PTT OR THIS SENSE LINE."
        )
    )
    story.append(P("7.3  S-Meter PWM", "sub"))
    story.append(
        P(
            "The adapter queries the 736 with CAT <b>F7</b> "
            "(the real radio command, not the dummy 847 "
            "<b>F7</b> on USB). Polling is every 3 s "
            "when idle, and about 633 ms (three times the CAT send time) "
            "when you have keyed in the last five minutes or the meter is "
            "above about S3. D3 PWM slews from the last sample to the new "
            "one across that window so an analog meter does not jump. Host "
            "CAT still wins the UART if a command arrives mid-poll."
        )
    )

    story.append(section("Section 8.  IN CASE OF DIFFICULTY"))
    story.append(
        om_table(
            ["Symptom", "Likely cause", "What to do"],
            [
                [
                    "No COM port",
                    "Driver, cable, or board",
                    "PIC: Microchip CDC INF. Arduino: 16U2 or CH340 driver. Try another USB cable that actually has data wires.",
                ],
                [
                    "Port exists, radio ignores CAT",
                    "Baud, DIN map, or CAT OFF",
                    "Host <b>must</b> be 4800 8N2 on Arduino. Confirm pin 2 = adapter TX, pin 4 = adapter RX, pin 1 = GND. Send CAT ON. Do not use pin 6.",
                ],
                [
                    "Simplex works, sat does not",
                    "Wrong dialect",
                    "A1 HIGH and software = FT-847. Native 736 opcodes are dropped in 847 mode.",
                ],
                [
                    "Panel locked, knobs dead",
                    "CAT is on",
                    "CAT OFF from the PC, or POWER-cycle the 736.",
                ],
                [
                    "HRD drops SAT on PTT",
                    "HRD bug",
                    "Use MOX, mic PTT, or footswitch. Do not use HRD\u2019s PTT in SAT.",
                ],
                [
                    "23 cm is 1 GHz off",
                    "847 digit limit",
                    "Tune 240 MHz in the program for 1240 MHz on the air.",
                ],
                [
                    "Arduino dies when RCA is plugged in",
                    "8 V on A0",
                    "Add the 22 kohm + zener. Do not continue on a damaged 328.",
                ],
                [
                    "S-meter PWM stuck",
                    "Poll lost the UART",
                    "Host CAT has priority. Idle the program; the next F7 should move D3.",
                ],
            ],
            [1.35 * inch, 1.45 * inch, w - 2.80 * inch],
        )
    )

    story.append(Spacer(1, 8))
    story.append(
        KeepTogether(
            [
                section("Appendix A.  CAT PIN MAP (USE THIS)"),
                P(
                    "Wire from the drawing below (Fig. 6, repeated) and this table. "
                    "AMSAT W6SHP: some FT-736R manuals and an old Yaesu application "
                    "note numbered the CAT shell sequentially; the pins are not. "
                    "Identify the keyway, then the numbered pins on the plug drawing."
                ),
                figure(
                    "fig-cat-din.png",
                    "Fig. 6 (repeated). CAT 6-pin DIN from the Yaesu Operating Manual. Pin functions 1\u20136 match AMSAT; do not recount around the shell.",
                    3.45 * inch,
                    max_h=2.15 * inch,
                ),
            ]
        )
    )
    story.append(
        om_table(
            ["Pin", "Radio name", "HS-736USB / Arduino"],
            [
                ["1", "GND", "GND"],
                ["2", "S.IN (serial data in)", "TX to the radio (D9)"],
                ["3", "BUSY", "PIC: no connect. Nano: D10 (J_CAT)"],
                ["4", "S.OUT (serial data out)", "RX from the radio (D8)"],
                ["5", "NC", "no connect"],
                ["6", "+13.8 V", "<b>do not connect</b>"],
            ],
            [0.7 * inch, 2.15 * inch, w - 2.85 * inch],
        )
    )
    story.append(Spacer(1, 8))
    story.append(
        P(
            "Looking into the radio\u2019s CAT socket (holes), the pattern is "
            "mirrored left-right. DATA IN/OUT on the 3.5 mm jack is a separate "
            "story: tip = received data from the radio, ring = transmit data "
            "to the radio, sleeve = GND. Some manuals swap tip and ring in "
            "the drawing. Full front- and rear-panel pinouts: "
            "<b>Arduino/docs/CONNECTIONS.md</b>."
        )
    )

    story.append(P("Appendix B.  SPECIFICATIONS OF THE INTERFACE", "sub"))
    spec_tbl = om_table(
            ["Item", "PIC HS-736USB", "Arduino Nano carrier"],
            [
                ["Host CAT", "FT-847, 4800 8N2 (CDC ignores baud)", "FT-847 or FT-736, baud <b>must</b> be 4800 8N2"],
                ["Radio CAT", "736, 4800 8N2 TTL, 50 ms inter-byte", "same, on J_CAT (pin 6 open)"],
                ["23 cm to PC", "shown as 240 MHz", "same"],
                ["Memories", "not supported; VFO only", "same"],
                ["Dialect switch", "847 only", "JP_A1 (A1\u2013GND), IRQ, CAT A5 73 36 sel FC"],
                ["PTT accessory", "\u2014", "J_AMP MOSFET; optional J_RCA sense"],
                ["Band decode", "\u2014", "J_BAND one-hot D4\u2013D7 (50/144/220/430)"],
                ["S-meter", "\u2014", "J_PWM D3, slewed, radio F7 poll"],
                ["Display", "\u2014", "J_LCD 16\u00d72 HD44780, RV1 contrast"],
                ["MCU board", "PIC18F14K50 on the PCB", "Arduino Nano in JP_L / JP_R"],
                ["CAD", "Hardware/Eagle", "Hardware/tscircuit"],
                ["Firmware", "Firmware/Source (GPL-3)", "Arduino/firmware (GPL-3)"],
            ],
            [1.35 * inch, 2.45 * inch, w - 3.80 * inch],
        )
    story.append(spec_tbl)
    story.append(Spacer(1, 4))
    story.append(
        P(
            "Opcode notes: <b>Arduino/docs/</b>. Original PIC booklet: "
            "<b>Manual/HS-736USB-Manual.pdf</b>. Figs. 1 and 3\u20137: Yaesu "
            "FT-736R Operating Manual (fair use). Fig. 9: HS-736USB booklet. "
            "Figs. 2 and 8: Ham Spot. Figs. 2A and 10: Nano carrier "
            "(Hardware/tscircuit).",
            "small",
        )
    )
    return story


def main():
    for required in (
        "fig-radio-34.png",
        "fig-front.png",
        "fig-rear.png",
        "fig-cat-din.png",
        "fig-sat-knobs.png",
        "fig-power-rear.png",
        "fig-hrd-connect.png",
        "fig-pcb.png",
        "fig-nano-pcb.png",
        "fig-nano-sch.png",
        "fig-faceplate.png",
    ):
        fig_path(required)

    doc = BaseDocTemplate(
        OUT,
        pagesize=letter,
        title="HS-736USB Operating Manual",
        author="Ham Spot Inc / hs736usb",
        subject="FT-736R CAT interface — Arduino / ATmega328P edition",
        creator="Manual/build_om.py",
    )
    cover_frame = Frame(0, 0, PAGE_W, PAGE_H, id="cover-frame", showBoundary=0)
    body_frame = Frame(
        MARGIN_L,
        MARGIN_B,
        COL_W,
        PAGE_H - MARGIN_T - MARGIN_B,
        id="body",
        showBoundary=0,
    )
    doc.addPageTemplates(
        [
            PageTemplate(id="cover", frames=[cover_frame], onPage=draw_cover),
            PageTemplate(id="body", frames=[body_frame], onPage=draw_interior),
        ]
    )
    story = [NextPageTemplate("body"), PageBreak()]
    story.extend(build_story())
    doc.build(story)
    print("wrote", OUT)


if __name__ == "__main__":
    main()
