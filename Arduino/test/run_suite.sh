#!/bin/sh
# Host mapper unit tests, then every known opcode vs cat_map state.
# With a device node: also send those opcodes to the adapter (and the 736 if wired).
set -e
ROOT=$(CDPATH= cd -- "$(dirname -- "$0")/.." && pwd)
cd "$ROOT"
# ROOT is the Arduino/ directory (firmware/ and test/ live here).

g++ -o /tmp/test_cat_map \
    test/test_cat_map.cpp firmware/cat_map.cpp firmware/cat_frame.cpp firmware/gpio_logic.cpp firmware/proto_debounce.cpp
/tmp/test_cat_map

if [ -n "$1" ]; then
    python3 test/radio_suite.py --port "$1"
else
    python3 test/radio_suite.py --local-only
fi
