#!/usr/bin/env python3
"""Pixel tests for the Scratchapixel rasterization build (TEST_PLAN §§1-6).

Reads the program's P3 PPM output and checks pixels, dimensions and
ordering. Independent of the C++ code: coverage expectations are
computed here from the projection + edge-function math, not copied
from the implementation. Tolerance: exact for solid/background
pixels, +/-1 per channel for interpolated colours.
Run: python3 tests/test_rasterizer.py  (builds ./rasterizer first)
"""
import math
import os
import subprocess
import sys

ROOT = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
BIN = os.path.join(ROOT, "rasterizer")
TMP = os.path.join(ROOT, "tests", "_out")
os.makedirs(TMP, exist_ok=True)

PASS = 0
FAIL = []


def check(name, cond, detail=""):
    global PASS
    if cond:
        PASS += 1
        print(f"  PASS {name}")
    else:
        FAIL.append(name)
        print(f"  FAIL {name} {detail}")


def run(scene, w=32, h=None, name="out"):
    out = os.path.join(TMP, f"{name}.ppm")
    r = subprocess.run([BIN, out, scene, str(w), str(h or w)],
                       capture_output=True, text=True)
    assert r.returncode == 0, f"{scene} exited {r.returncode}: {r.stderr}"
    return read_ppm(out), out


def read_ppm(path):
    with open(path) as f:
        toks = f.read().split()
    assert toks[0] == "P3", f"bad magic {toks[0]}"
    w, h, mx = int(toks[1]), int(toks[2]), int(toks[3])
    vals = [int(t) for t in toks[4:]]
    assert len(vals) == w * h * 3, f"pixel count {len(vals)//3} != {w*h}"
    assert mx == 255
    px = [(vals[i], vals[i + 1], vals[i + 2]) for i in range(0, len(vals), 3)]
    return w, h, px


def pix(img, x, y):
    w, h, px = img
    return px[y * w + x]


def edge(a, b, p):
    return (p[0] - a[0]) * (b[1] - a[1]) - (p[1] - a[1]) * (b[0] - a[0])


def inside(tri, px, py):
    """Independent top-left-rule inside test, pixel centre, winding normalised."""
    v = list(tri)
    area = edge(v[0], v[1], v[2])
    if area == 0:
        return False
    if area < 0:
        v[1], v[2] = v[2], v[1]
        area = -area
    p = (px + 0.5, py + 0.5)
    edges = [(v[1], v[2]), (v[2], v[0]), (v[0], v[1])]
    for a, b in edges:
        e = edge(a, b, p)
        if e < 0:
            return False
        if e == 0:
            dx, dy = b[0] - a[0], b[1] - a[1]
            if not ((dy == 0 and dx > 0) or dy < 0):
                return False
    return True


def count_colour(img, colour):
    return sum(1 for p in img[2] if p == colour)


def main():
    subprocess.run(["make", "-C", ROOT], check=True, capture_output=True)

    print("§1 PPM format / empty / bad args")
    img, path = run("default", 64, name="default")
    check("header P3 64x64 max 255", img[0] == 64 and img[1] == 64)
    # Row-major, top row first: the clip triangle occupies the
    # top-left corner (row 0 drawn) and not the bottom-right
    # corner (pixel (63,63) black). Note the bottom row's pixel
    # (0,63) IS legitimately drawn: it lies on the diagonal edge,
    # which is a left edge in normalised winding, so the top-left
    # rule assigns it to this triangle.
    imgc, _ = run("clip", 64, name="clip64")
    check("row-major top-first (top-left drawn, bottom-right black)",
          pix(imgc, 0, 0) == (200, 30, 30) and pix(imgc, 63, 63) == (0, 0, 0))
    img, _ = run("empty", 16, name="empty")
    check("empty scene all black", all(p == (0, 0, 0) for p in img[2]))
    bad = os.path.join(TMP, "bad.ppm")
    if os.path.exists(bad):
        os.remove(bad)
    r = subprocess.run([BIN], capture_output=True, text=True)
    check("no args: non-zero + usage on stderr, no file",
          r.returncode != 0 and "usage" in r.stderr and not os.path.exists(bad))
    r = subprocess.run([BIN, bad, "default", "0", "10"], capture_output=True, text=True)
    check("width 0: non-zero, no file",
          r.returncode != 0 and not os.path.exists(bad))
    r = subprocess.run([BIN, bad, "nope"], capture_output=True, text=True)
    check("unknown scene: non-zero, no file",
          r.returncode != 0 and not os.path.exists(bad))
    r = subprocess.run([BIN, os.path.join(TMP, "nope_dir", "x.ppm"), "default"],
                       capture_output=True, text=True)
    check("unwritable path: non-zero", r.returncode != 0)

    print("§2 coverage / bounding box / clipping / shared edge")
    img, _ = run("solid", 32, name="solid")
    tri = [(8, 8), (24, 8), (8, 24)]
    mism = [(x, y) for y in range(32) for x in range(32)
            if (pix(img, x, y) == (200, 30, 30)) != inside(tri, x, y)]
    check("solid triangle matches independent edge test exactly", not mism,
          f"mismatches={mism[:8]}")
    check("solid corner pixel outside bbox is background",
          pix(img, 31, 31) == (0, 0, 0) and pix(img, 0, 31) == (0, 0, 0))
    img, _ = run("clip", 32, name="clip")
    check("clipping: no crash, some pixels drawn, corner covered",
          count_colour(img, (200, 30, 30)) > 0 and pix(img, 0, 0) == (200, 30, 30))
    img, _ = run("quad", 32, name="quad")
    quad = [(8, 8), (24, 24)]
    gap = blend = 0
    for y in range(32):
        for x in range(32):
            # A pixel is in the square iff inside either half-triangle.
            t1 = [(8, 8), (24, 8), (24, 24)]
            t2 = [(8, 8), (24, 24), (8, 24)]
            in_sq = inside(t1, x, y) or inside(t2, x, y)
            c = pix(img, x, y)
            if in_sq and c == (0, 0, 0):
                gap += 1
            if not in_sq and c != (0, 0, 0):
                blend += 1
            if in_sq and c not in ((255, 0, 0), (0, 255, 0)):
                blend += 1
    check("shared edge: quad covered, no gaps, no blends, one colour each",
          gap == 0 and blend == 0, f"gap={gap} bad={blend}")
    # Shared diagonal pixels: each must be exactly red or green.
    diag = [pix(img, d, d) for d in range(9, 24)]
    check("shared diagonal pixels are red or green only",
          all(c in ((255, 0, 0), (0, 255, 0)) for c in diag))

    print("§3 barycentric interpolation")
    img, _ = run("default", 64, name="default64")
    # Default triangle (64x64, z=-2 so ndc = cam/2):
    # raster verts (32,22.4) red, (22.4,40) green, (41.6,40) blue.
    # True centroid is (32, 34.1), which is NOT a pixel centre, so
    # TEST_PLAN's "centroid pixel = average" holds only approximately
    # here. Instead assert the nearest pixel centre (32.5, 34.5)
    # against its independently computed barycentric colour (+/-1).
    c = pix(img, 32, 34)
    v = [(32, 22.4), (22.4, 40), (41.6, 40)]
    p = (32.5, 34.5)
    area = edge(v[0], v[1], v[2])
    ws = [edge(v[1], v[2], p) / area, edge(v[2], v[0], p) / area,
          edge(v[0], v[1], p) / area]
    exp = (round(ws[0] * 255), round(ws[1] * 255), round(ws[2] * 255))
    check("centroid-area colour = independent barycentric within +/-1",
          all(abs(c[i] - exp[i]) <= 1 for i in range(3)),
          f"got {c} expected {exp}")
    # Near the red vertex (top): red dominant.
    c = pix(img, 32, 24)
    check("near red vertex: red dominant", c[0] > 180 and c[1] < 80 and c[2] < 80,
          f"got {c}")
    check("near green vertex: green dominant",
          pix(img, 24, 38)[1] > 180, f"got {pix(img, 24, 38)}")
    check("near blue vertex: blue dominant",
          pix(img, 40, 38)[2] > 180, f"got {pix(img, 40, 38)}")

    print("§4 depth ordering")
    a, pa = run("depth-near-first", 32, name="dn")
    b, pb = run("depth-far-first", 32, name="df")
    check("nearer wins regardless of submission order (byte-identical)",
          open(pa, "rb").read() == open(pb, "rb").read())
    check("overlap region is nearer (red) colour",
          pix(a, 12, 12) == (255, 0, 0))
    img, _ = run("depth-equal", 32, name="de")
    check("equal depth: first triangle wins (red, strict <)",
          pix(img, 12, 12) == (255, 0, 0))
    img2, _ = run("depth-equal", 32, name="de2")
    check("equal depth deterministic (byte-identical rerun)",
          open(os.path.join(TMP, "de.ppm"), "rb").read() ==
          open(os.path.join(TMP, "de2.ppm"), "rb").read())

    print("§5 projection")
    n, _ = run("persp-near", 64, name="pn")
    f, _ = run("persp-far", 64, name="pf")
    cn, cf = count_colour(n, (200, 30, 30)), count_colour(f, (200, 30, 30))
    check("perspective size: near covers ~4x far (area ratio)",
          3.0 < cn / cf < 5.0, f"near={cn} far={cf}")
    img, _ = run("offcentre", 64, name="off")
    pts = [(x, y) for y in range(64) for x in range(64)
           if pix(img, x, y) != (0, 0, 0)]
    cx = sum(p[0] for p in pts) / len(pts)
    cy = sum(p[1] for p in pts) / len(pts)
    check("off-centre triangle centroid right/up of centre",
          cx > 32 and cy < 32, f"centroid=({cx:.1f},{cy:.1f})")
    img, _ = run("behind", 32, name="behind")
    check("behind camera / inside near plane: no pixels",
          all(p == (0, 0, 0) for p in img[2]))
    img, _ = run("crossing", 32, name="cross")
    check("near-plane crossing: no garbage (discarded whole, all black)",
          all(p == (0, 0, 0) for p in img[2]))

    print("§6 determinism / reference")
    _, p1 = run("default", 64, name="det1")
    _, p2 = run("default", 64, name="det2")
    check("same scene twice byte-identical",
          open(p1, "rb").read() == open(p2, "rb").read())
    # Reference PPM kept for review.
    import shutil
    shutil.copy(p1, os.path.join(ROOT, "tests", "reference_default_64.ppm"))
    check("reference PPM saved", True)

    print(f"\n{PASS} passed, {len(FAIL)} failed: {FAIL}")
    return 1 if FAIL else 0


if __name__ == "__main__":
    sys.exit(main())
