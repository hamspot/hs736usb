"""Every known FT-847 opcode this adapter handles, with representative params.

Not a cartesian product of all 4-byte BCD values (that is 2^32). Coverage:
all opcodes in cat_map, every 847 mode byte, every in-range CTCSS index,
band-edge frequencies, HRD and Hamlib repeater forms, drops, and reads.
"""

MODES = [
    (0x00, "LSB"),
    (0x01, "USB"),
    (0x02, "CW"),
    (0x03, "CWR"),
    (0x04, "AM"),
    (0x08, "FM"),
    (0x82, "CWN"),
    (0x83, "CWRN"),
    (0x84, "AMN"),
    (0x88, "FMN"),
]

FREQS = [
    ([0x05, 0x00, 0x00, 0x00], "50.0000"),
    ([0x14, 0x40, 0x00, 0x00], "144.0000"),
    ([0x14, 0x50, 0x00, 0x00], "145.0000"),
    ([0x14, 0x79, 0x99, 0x90], "147.9990"),
    ([0x22, 0x00, 0x00, 0x00], "220.0000"),
    ([0x43, 0x20, 0x00, 0x00], "432.0000"),
    ([0x43, 0x50, 0x00, 0x00], "435.0000"),
    ([0x44, 0x00, 0x00, 0x00], "440.0000"),
    ([0x24, 0x00, 0x00, 0x00], "1240 as 240"),
    ([0x30, 0x00, 0x00, 0x00], "1300 as 300"),
]


def b5(p0, p1, p2, p3, op):
    return bytes((p0, p1, p2, p3, op))


def all_cases():
    cases = []
    cases.append(("CAT ON", b5(0, 0, 0, 0, 0x00), 3, True))
    for freq, name in FREQS:
        cases.append((f"set main {name}", bytes(freq + [0x01]), 1, True))
    for md, name in MODES:
        cases.append((f"mode main {name}", b5(md, 0, 0, 0, 0x07), 1, True))
    cases.append(("read main 03", b5(0, 0, 0, 0, 0x03), 0, False))
    cases.append(("read RX E7", b5(0, 0, 0, 0, 0xE7), 0, False))
    cases.append(("read TX F7", b5(0, 0, 0, 0, 0xF7), 0, False))
    cases.append(("sat ON", b5(0, 0, 0, 0, 0x4E), 1, True))
    for freq, name in FREQS:
        cases.append((f"set sat RX {name}", bytes(freq + [0x11]), 1, True))
        cases.append((f"set sat TX {name}", bytes(freq + [0x21]), 1, True))
    for md, name in MODES:
        cases.append((f"mode sat RX {name}", b5(md, 0, 0, 0, 0x17), 1, True))
        cases.append((f"mode sat TX {name}", b5(md, 0, 0, 0, 0x27), 1, True))
    cases.append(("read sat RX 13", b5(0, 0, 0, 0, 0x13), 0, False))
    cases.append(("read sat TX 23", b5(0, 0, 0, 0, 0x23), 0, False))
    cases.append(("shift HRD minus", b5(0x09, 0, 0, 0, 0x09), 1, True))
    cases.append(("shift HRD plus", b5(0x49, 0, 0, 0, 0x09), 1, True))
    cases.append(("shift HRD simplex", b5(0x89, 0, 0, 0, 0x09), 1, True))
    cases.append(("shift Hamlib plus", b5(0, 0, 0, 0, 0x49), 1, True))
    cases.append(("shift Hamlib simplex", b5(0, 0, 0, 0, 0x89), 1, True))
    cases.append(("offset F9 600 kHz", b5(0x00, 0x60, 0x00, 0x00, 0xF9), 1, True))
    cases.append(("CTCSS ENC/DEC", b5(0x2A, 0, 0, 0, 0x0A), 1, True))
    cases.append(("CTCSS ENC", b5(0x4A, 0, 0, 0, 0x0A), 1, True))
    cases.append(("CTCSS off", b5(0x8A, 0, 0, 0, 0x0A), 1, True))
    cases.append(("DCS drop", b5(0x0A, 0, 0, 0, 0x0A), 0, False))
    for i in range(38):
        cases.append((f"tone idx {i}", b5(i, 0, 0, 0, 0x0B), 1, True))
    cases.append(("tone OOB drop", b5(0x40, 0, 0, 0, 0x0B), 0, False))
    cases.append(("unknown drop", b5(0, 0, 0, 0, 0x99), 0, False))
    cases.append(("sat OFF", b5(0, 0, 0, 0, 0x8E), 2, True))
    # PTT last: keys the transmitter on a live 736.
    cases.append(("PTT ON", b5(0, 0, 0, 0, 0x08), 1, True))
    cases.append(("PTT OFF", b5(0, 0, 0, 0, 0x88), 1, True))
    cases.append(("CAT OFF", b5(0, 0, 0, 0, 0x80), 1, True))
    return cases


DEBUG_CMD = bytes((0xA5, 0x73, 0x36, 0x00, 0xFE))
