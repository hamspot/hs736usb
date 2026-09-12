#!/usr/bin/env python3
"""Walk every known CAT opcode; compare Arduino state dump to local cat_map.

Usage:
  test/run_suite.sh                         # host mapper + local opcode walk
  test/run_suite.sh /dev/ttyUSB0            # also the programmed adapter
  python3 test/radio_suite.py --port /dev/ttyUSB0

On a live FT-736R this sends real CAT (including a brief PTT ON then OFF).
Set VFO + sat VFO first. The 736 CAT LED should light.
"""

from __future__ import annotations

import argparse
import os
import subprocess
import sys
import time

from opcodes import DEBUG_CMD, all_cases

ROOT = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
STEP_BIN = "/tmp/hs736_cat_step"


def compile_step() -> None:
    src = os.path.join(ROOT, "test", "cat_step.cpp")
    mapper = os.path.join(ROOT, "firmware", "cat_map.cpp")
    gpio = os.path.join(ROOT, "firmware", "gpio_logic.cpp")
    cmd = ["g++", "-o", STEP_BIN, src, mapper, gpio]
    subprocess.check_call(cmd)


def parse_dump(line: str) -> bytes:
    parts = line.strip().split()
    if len(parts) != 32:
        raise ValueError(f"dump length {len(parts)}: {line!r}")
    return bytes(int(p, 16) for p in parts)


def hex5(b: bytes) -> str:
    return " ".join(f"{x:02X}" for x in b)


def open_tty(path: str) -> int:
    import termios

    fd = os.open(path, os.O_RDWR | os.O_NOCTTY | os.O_NONBLOCK)
    t = termios.tcgetattr(fd)
    iflag, oflag, cflag, lflag, ispeed, ospeed, cc = t
    cflag &= ~(termios.CSIZE | termios.PARENB | getattr(termios, "CRTSCTS", 0))
    cflag |= termios.CS8 | termios.CSTOPB | termios.CREAD | termios.CLOCAL
    lflag &= ~(termios.ECHO | termios.ICANON | termios.ISIG | termios.IEXTEN)
    iflag &= ~(termios.IXON | termios.IXOFF | termios.ICRNL | termios.INLCR)
    oflag &= ~termios.OPOST
    cc[termios.VMIN] = 0
    cc[termios.VTIME] = 0
    t = [iflag, oflag, cflag, lflag, termios.B4800, termios.B4800, cc]
    termios.tcsetattr(fd, termios.TCSANOW, t)
    termios.tcflush(fd, termios.TCIOFLUSH)
    return fd


def tty_write(fd: int, data: bytes) -> None:
    sent = 0
    while sent < len(data):
        n = os.write(fd, data[sent:])
        if n <= 0:
            raise OSError("short serial write")
        sent += n


def tty_read_exact(fd: int, n: int, timeout_s: float) -> bytes:
    import select

    buf = bytearray()
    deadline = time.monotonic() + timeout_s
    while len(buf) < n:
        remain = deadline - time.monotonic()
        if remain <= 0:
            raise TimeoutError(f"serial read {len(buf)}/{n}")
        r, _, _ = select.select([fd], [], [], remain)
        if not r:
            continue
        chunk = os.read(fd, n - len(buf))
        if chunk:
            buf.extend(chunk)
    return bytes(buf)


def radio_wait(n_radio: int) -> None:
    # 50 ms between radio bytes (firmware) + margin.
    time.sleep(0.05 * 5 * max(n_radio, 0) + 0.08)


def run_local(cases) -> int:
    compile_step()
    proc = subprocess.Popen(
        [STEP_BIN],
        stdin=subprocess.PIPE,
        stdout=subprocess.PIPE,
        stderr=subprocess.PIPE,
        text=True,
        bufsize=1,
    )
    fail = 0
    assert proc.stdin is not None and proc.stdout is not None
    for name, cmd, _n_radio, _live in cases:
        proc.stdin.write(hex5(cmd) + "\n")
        proc.stdin.flush()
        line = proc.stdout.readline()
        try:
            dump = parse_dump(line)
        except ValueError as e:
            print(f"FAIL {name}: {e}")
            fail += 1
            continue
        if dump[0] != 0xA5:
            print(f"FAIL {name}: magic {dump[0]:02X}")
            fail += 1
            continue
        flags = dump[1]
        if name == "CAT ON" and (flags & 0x01) == 0:
            print(f"FAIL {name}: cat_on not set")
            fail += 1
            continue
        if name == "PTT ON" and (flags & 0x02) == 0:
            print(f"FAIL {name}: ptt not set")
            fail += 1
            continue
        if name == "sat ON" and (flags & 0x04) == 0:
            print(f"FAIL {name}: sat not set")
            fail += 1
            continue
        if name == "CAT OFF" and (flags & 0x01) != 0:
            print(f"FAIL {name}: cat_on still set")
            fail += 1
            continue
        print(f"ok  {name}")
    proc.stdin.close()
    proc.wait(timeout=5)
    return fail


def run_serial(port: str, cases) -> int:
    compile_step()
    proc = subprocess.Popen(
        [STEP_BIN],
        stdin=subprocess.PIPE,
        stdout=subprocess.PIPE,
        stderr=subprocess.PIPE,
        text=True,
        bufsize=1,
    )
    assert proc.stdin is not None and proc.stdout is not None
    fd = open_tty(port)
    # Uno/Nano auto-reset on open.
    time.sleep(2.5)
    fail = 0
    try:
        for name, cmd, n_radio, _live in cases:
            proc.stdin.write(hex5(cmd) + "\n")
            proc.stdin.flush()
            expect = parse_dump(proc.stdout.readline())
            tty_write(fd, cmd)
            radio_wait(n_radio)
            tty_write(fd, DEBUG_CMD)
            time.sleep(0.05)
            try:
                got = tty_read_exact(fd, 32, 2.0)
            except TimeoutError as e:
                print(f"FAIL {name}: {e}")
                fail += 1
                continue
            if got != expect:
                print(f"FAIL {name}")
                print(f"  got {hex5(got)}")
                print(f"  exp {hex5(expect)}")
                fail += 1
            else:
                print(f"ok  {name}")
            if name == "PTT ON":
                # Do not leave the 736 in TX if the next command fails.
                time.sleep(0.05)
    finally:
        try:
            tty_write(fd, bytes((0, 0, 0, 0, 0x88)))
            time.sleep(0.3)
            tty_write(fd, bytes((0, 0, 0, 0, 0x80)))
        except OSError:
            pass
        os.close(fd)
        proc.stdin.close()
        proc.wait(timeout=5)
    return fail


def main() -> int:
    p = argparse.ArgumentParser(description="HS-736 opcode suite vs internal state dump")
    p.add_argument("--port", help="adapter serial device (4800 8N2)")
    p.add_argument("--local-only", action="store_true", help="do not open serial")
    args = p.parse_args()
    cases = all_cases()
    print(f"{len(cases)} opcode cases")
    fail = run_local(cases)
    if fail:
        print(f"local walk {fail} failure(s)")
        return 1
    print("local walk ok")
    if args.local_only or not args.port:
        return 0
    print(f"serial {args.port} (live radio if CAT is wired)")
    fail = run_serial(args.port, cases)
    if fail:
        print(f"serial {fail} failure(s)")
        return 1
    print("serial walk ok")
    return 0


if __name__ == "__main__":
    sys.exit(main())
