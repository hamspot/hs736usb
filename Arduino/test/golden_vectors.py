#!/usr/bin/env python3
"""Human-readable copy of the C golden vectors. The C test is authoritative."""

VECTORS = [
    ("CAT ON", [0, 0, 0, 0, 0x00],
     [[0, 0, 0, 0, 0x00], [0x14, 0x50, 0, 0, 0x01], [0x08, 0, 0, 0, 0x07]],
     None),
    ("sat RX freq", [0x14, 0x50, 0, 0, 0x11], [[0x14, 0x50, 0, 0, 0x1E]], None),
    ("sat TX freq", [0x43, 0x50, 0, 0, 0x21], [[0x43, 0x50, 0, 0, 0x2E]], None),
    ("sat on", [0, 0, 0, 0, 0x4E], [[0, 0, 0, 0, 0x0E]], None),
    ("23cm set", [0x24, 0, 0, 0, 0x01], [[0xC4, 0, 0, 0, 0x01]], None),
    ("DCS drop", [0x0A, 0, 0, 0, 0x0A], [], None),
]


def main():
    print(f"{len(VECTORS)} documented vectors; run test/test_cat_map.cpp")


if __name__ == "__main__":
    main()
