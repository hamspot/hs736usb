# Tests

One command for unit tests plus every known opcode vs the internal state dump:

```
test/run_suite.sh
```

Against a programmed adapter (and the FT-736R if CAT is wired). This sends real CAT, including a short PTT ON then OFF:

```
test/run_suite.sh /dev/ttyUSB0
```

Host mapper only:

```
g++ -o /tmp/test_cat_map test/test_cat_map.cpp firmware/cat_map.cpp firmware/cat_frame.cpp firmware/gpio_logic.cpp firmware/proto_debounce.cpp
/tmp/test_cat_map
```

On a programmed Uno/Nano, without the FT-736R connected:

```
rigctl -m 1001 -r /dev/ttyUSB0 -s 4800
```

Hamlib model **1001** is FT-847. Set frequency and mode, then `f` / `m` should return the cache. `t` (PTT) should not time out.

With the radio: VFO + sat VFO, then the 736 CAT LED should light on CAT ON. See `docs/WIRING.md`.
