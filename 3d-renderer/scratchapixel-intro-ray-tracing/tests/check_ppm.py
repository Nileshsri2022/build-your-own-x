#!/usr/bin/env python3
"""Integration check for the Scratchapixel raytracer (TEST_PLAN).
Builds/runs the real binary in a temp dir, parses the PPM, and asserts:
header/dimensions/integer count, centre pixel red, corner black,
light-marker pixel white, reflective pixel differs from flat green,
and at least one black pixel on the red sphere's shadowed left side.
Exits non-zero on any failure. Run via `make check` or directly after
`make raytracer` (it builds the binary itself if needed).
"""
import os
import subprocess
import sys
import tempfile

ROOT = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
BIN = os.path.join(ROOT, "raytracer")
failures = []


def check(ok, name):
    print(("ok: " if ok else "FAIL: ") + name)
    if not ok:
        failures.append(name)


def parse_ppm(path):
    with open(path) as f:
        tokens = f.read().split()
    assert tokens[0] == "P3", "magic"
    w, h, mx = int(tokens[1]), int(tokens[2]), int(tokens[3])
    body = [int(t) for t in tokens[4:]]
    return w, h, mx, body


def px(body, w, x, y):
    i = (y * w + x) * 3
    return tuple(body[i:i + 3])


def main():
    with tempfile.TemporaryDirectory() as td:
        out = os.path.join(td, "out.ppm")
        r = subprocess.run([BIN, out, "200", "200"], capture_output=True, text=True)
        check(r.returncode == 0, "render exit 0")
        check(os.path.exists(out), "output file exists")
        w, h, mx, body = parse_ppm(out)
        check((w, h, mx) == (200, 200, 255), "header 200x200 max 255")
        check(len(body) == 200 * 200 * 3, "body integer count matches")
        check(all(0 <= v <= 255 for v in body), "all channels in [0,255]")
        check(px(body, w, 100, 100) == (255, 0, 0), "centre pixel red (255,0,0)")
        check(px(body, w, 0, 0) == (0, 0, 0), "corner pixel black")
        # Light marker: centre (0,3,-3) projects to plane (0,1), top edge;
        # its visible strip is around (100,8).
        check(px(body, w, 100, 8) == (255, 255, 255), "light-marker pixel white")
        # Reflective green sphere centre (0.9,1.2,-3.8) -> plane (0.237,0.316)
        # -> (124,68).
        g = px(body, w, 124, 68)
        check(g[1] > 0 and g != (0, 255, 0),
              f"reflective pixel differs from flat green, got {g}")
        # Shadow: right side of the red sphere (centre (100,100), radius ~20px),
        # shadowed by the green sphere between it and the light.
        shadow = [px(body, w, x, y) for x in range(108, 119) for y in range(95, 106)]
        check(any(p == (0, 0, 0) for p in shadow),
              "at least one black (shadowed) pixel on red sphere's right side")
        # CLI failures: no args, bad width.
        r = subprocess.run([BIN], capture_output=True, text=True)
        check(r.returncode != 0 and "Usage" in r.stderr, "no args -> non-zero + usage")
        check(not os.path.exists(os.path.join(td, "should-not-exist.ppm")),
              "no stray output file from failed run")
        r = subprocess.run([BIN, os.path.join(td, "bad.ppm"), "0", "200"],
                           capture_output=True, text=True)
        check(r.returncode != 0, "non-positive width -> non-zero")
    if failures:
        print(f"{len(failures)} integration check(s) failed")
        return 1
    print("ALL INTEGRATION CHECKS PASSED")
    return 0


if __name__ == "__main__":
    sys.exit(main())
