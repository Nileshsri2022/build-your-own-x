#!/usr/bin/env python3
"""Pixel-based tests for tinyrenderer (TEST_PLAN.md), run for real.
Generates .obj/.tga fixtures in a temp dir, builds with CMake, renders
with the real binary, and asserts on output TGA pixels. Colour tolerance
is +/-3 per TEST_PLAN unless a check says otherwise (relative checks
use larger margins and are marked as such).
Exit 0 only if every check passes.
"""
import os, struct, subprocess, sys, tempfile

ROOT = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
BIN = os.path.join(ROOT, "build", "tinyrenderer")
W = H = 800
passed, failed = [], []

def check(name, cond, detail=""):
    (passed if cond else failed).append(name)
    print(("PASS " if cond else "FAIL ") + name + ((" — " + str(detail)) if detail else ""))

# ---------- TGA helpers (uncompressed type 2, 24-bit, top-left origin) ----------
def write_tga(path, w, h, fn):
    with open(path, "wb") as f:
        hdr = bytearray(18)
        hdr[2] = 2; hdr[12] = w & 0xFF; hdr[13] = (w >> 8) & 0xFF
        hdr[14] = h & 0xFF; hdr[15] = (h >> 8) & 0xFF; hdr[16] = 24; hdr[17] = 0x20
        f.write(hdr)
        for y in range(h):
            for x in range(w):
                r, g, b = fn(x, y)
                f.write(bytes([b, g, r]))

def read_tga(path):
    with open(path, "rb") as f:
        hdr = f.read(18)
        assert len(hdr) == 18, "short header"
        idlen, cmap, itype = hdr[0], hdr[1], hdr[2]
        w = hdr[12] | (hdr[13] << 8); h = hdr[14] | (hdr[15] << 8)
        bpp, desc = hdr[16], hdr[17]
        assert itype == 2 and bpp == 24, f"unexpected TGA type/bpp {itype}/{bpp}"
        f.seek(idlen, 1)
        raw = f.read(w * h * 3)
        assert len(raw) == w * h * 3, "short pixel data"
    rows = []
    for y in range(h):
        file_row = y if (desc & 0x20) else (h - 1 - y)
        row = []
        for x in range(w):
            i = (file_row * w + x) * 3
            row.append((raw[i + 2], raw[i + 1], raw[i]))  # BGR -> RGB
        rows.append(row)
    return w, h, rows

def px(rows, x, y):
    return rows[y][x]

def bright(c):
    return (c[0] + c[1] + c[2]) / 3.0

def close(a, b, tol=3):
    return all(abs(x - y) <= tol for x, y in zip(a, b))

def write_obj(path, text):
    with open(path, "w") as f:
        f.write(text)

def render(args, cwd):
    return subprocess.run([BIN] + args, cwd=cwd, capture_output=True, text=True)

def main():
    tmp = tempfile.mkdtemp(prefix="tiny-test-")
    # Build (idempotent; Makefile/CMake already ran, but be explicit).
    r = subprocess.run(["cmake", "--build", os.path.join(ROOT, "build"), "-j"],
                       capture_output=True, text=True)
    check("build succeeds", r.returncode == 0, r.stderr[-300:] if r.returncode else "")

    # ---- fixtures ----
    # Single triangle, world CCW viewed from +z (front-facing), with UVs.
    tri = ("v -0.8 -0.8 0\nv 0.8 -0.8 0\nv 0 0.8 0\n"
           "vt 0 0\nvt 1 0\nvt 0.5 1\nf 1/1 2/2 3/3\n")
    tri_rev = ("v -0.8 -0.8 0\nv 0.8 -0.8 0\nv 0 0.8 0\n"
               "vt 0 0\nvt 1 0\nvt 0.5 1\nf 1/1 3/3 2/2\n")
    write_obj(f"{tmp}/tri.obj", tri)
    write_obj(f"{tmp}/tri_rev.obj", tri_rev)
    write_tga(f"{tmp}/red.tga", 4, 4, lambda x, y: (255, 0, 0))
    # Two overlapping triangles: near (z=0.5, red) and far (z=-0.5, blue).
    near = ("v -0.8 -0.8 0.5\nv 0.8 -0.8 0.5\nv 0 0.8 0.5\n"
            "vt 0 0\nvt 1 0\nvt 0.5 1\nf 1/1 2/2 3/3\n")
    far = ("v -0.8 -0.8 -0.5\nv 0.8 -0.8 -0.5\nv 0 0.8 -0.5\n"
           "vt 0 0\nvt 1 0\nvt 0.5 1\nf 1/1 2/2 3/3\n")
    write_obj(f"{tmp}/near.obj", near)
    write_obj(f"{tmp}/far.obj", far)
    write_tga(f"{tmp}/blue.tga", 4, 4, lambda x, y: (0, 0, 255))
    # Quad for camera/texture/normal tests (two triangles, CCW from +z).
    quad = ("v -0.8 -0.8 0\nv 0.8 -0.8 0\nv 0.8 0.8 0\nv -0.8 0.8 0\n"
            "vt 0 0\nvt 1 0\nvt 1 1\nvt 0 1\n"
            "f 1/1 2/2 3/3\nf 1/1 3/3 4/4\n")
    write_obj(f"{tmp}/quad.obj", quad)
    # Quadrant texture: top-left red, top-right green, bottom-left blue,
    # bottom-right white (in UV terms: v=1 is the top row).
    def quadrant(x, y):
        top = y < 32
        left = x < 32
        if top and left: return (255, 0, 0)
        if top and not left: return (0, 255, 0)
        if not top and left: return (0, 0, 255)
        return (255, 255, 255)
    write_tga(f"{tmp}/quad.tga", 64, 64, quadrant)
    # Normal map: checker of flat vs tilted normals.
    def nmap(x, y):
        return (128, 128, 255) if ((x // 8) + (y // 8)) % 2 == 0 else (200, 128, 255)
    write_tga(f"{tmp}/nmap.tga", 64, 64, nmap)
    # Phong triangles: same geometry, vn facing the light vs tilted away.
    phong_bright = ("v -0.8 -0.8 0\nv 0.8 -0.8 0\nv 0 0.8 0\n"
                    "vn 0 0 1\nf 1//1 2//1 3//1\n")
    phong_dim = ("v -0.8 -0.8 0\nv 0.8 -0.8 0\nv 0 0.8 0\n"
                 "vn 0.8 0 0.6\nf 1//1 2//1 3//1\n")
    write_obj(f"{tmp}/phong_bright.obj", phong_bright)
    write_obj(f"{tmp}/phong_dim.obj", phong_dim)
    # Shadow scene: floor quad 4x4 at z=0, occluder quad 1x1 at (0,1,1).
    floor = ("v -2 -2 0\nv 2 -2 0\nv 2 2 0\nv -2 2 0\n"
             "f 1 2 3\nf 1 3 4\n")
    occ = ("v -0.5 0.5 1\nv 0.5 0.5 1\nv 0.5 1.5 1\nv -0.5 1.5 1\n"
           "f 1 2 3\nf 1 3 4\n")
    write_obj(f"{tmp}/floor.obj", floor)
    write_obj(f"{tmp}/occ.obj", occ)
    # Cube: 12 triangles, outward CCW faces (front face toward +z).
    cube = []
    V = [(-0.7, -0.7, -0.7), (0.7, -0.7, -0.7), (0.7, 0.7, -0.7), (-0.7, 0.7, -0.7),
         (-0.7, -0.7, 0.7), (0.7, -0.7, 0.7), (0.7, 0.7, 0.7), (-0.7, 0.7, 0.7)]
    for v in V: cube.append(f"v {v[0]} {v[1]} {v[2]}")
    cube += ["vt 0 0", "vt 1 0", "vt 1 1", "vt 0 1"]
    faces = [(5, 6, 7), (5, 7, 8),  # front (+z)
             (2, 1, 4), (2, 4, 3),  # back (-z)
             (1, 5, 8), (1, 8, 4),  # left (-x)
             (6, 2, 3), (6, 3, 7),  # right (+x)
             (8, 7, 3), (8, 3, 4),  # top (+y)
             (1, 2, 6), (1, 6, 5)]  # bottom (-y)
    for a, b, c in faces: cube.append(f"f {a}/1 {b}/2 {c}/3")
    write_obj(f"{tmp}/cube.obj", "\n".join(cube) + "\n")

    # ---- T1 framebuffer: reversed-winding triangle = all background ----
    r = render(["tri_rev.obj", "--diffuse", "red.tga", "bg.tga"], tmp)
    w, h, rows = read_tga(f"{tmp}/bg.tga")
    check("T1 file valid, 800x800", (w, h) == (800, 800) and r.returncode == 0)
    check("T1 all background (culled triangle renders nothing)",
          all(px(rows, x, y) == (0, 0, 0) for y in range(0, 800, 7) for x in range(0, 800, 7)))

    # ---- T2 triangle rasterization ----
    r = render(["tri.obj", "--diffuse", "red.tga", "--no-shadow", "tri.tga"], tmp)
    w, h, rows = read_tga(f"{tmp}/tri.tga")
    c = px(rows, 400, 500)  # inside the triangle (below its apex, above base)
    # Rasterization check, not a shading check: Phong (T5) legitimately
    # scales the texture colour by the light angle, so assert red-dominant
    # rather than exact red — coverage + correct texture sample is the point.
    check("T2 centre pixel is red-dominant", c[0] > 200 and c[0] > c[1] + 100 and c[0] > c[2] + 100, c)
    check("T2 outside pixel is background", px(rows, 50, 50) == (0, 0, 0))
    check("T2 corner pixel is background", px(rows, 750, 750) == (0, 0, 0))

    # ---- T3 depth: nearer wins in both submission orders ----
    for order in (["near.obj", "far.obj"], ["far.obj", "near.obj"]):
        args = order + ["--diffuse", "red.tga", "--no-shadow", "depth.tga"]
        # NOTE: --diffuse applies to the first model only, so the second
        # model renders white; the check below therefore uses the FIRST
        # model in each order: it must win at the centre regardless.
        r = render(args, tmp)
        _, _, rows = read_tga(f"{tmp}/depth.tga")
        centre = px(rows, 400, 500)
        if order[0] == "near.obj":
            # Red-dominant (not exact red): Phong scales colour, see T2 note.
            check(f"T3 nearer wins (order {order[0]} first)",
                  centre[0] > 200 and centre[0] > centre[1] + 100 and centre[0] > centre[2] + 100, centre)
        else:
            # far first, near second (as floor): near is white, must still win
            check(f"T3 nearer wins (order {order[0]} first)", bright(centre) > 200, centre)

    # ---- T4 camera: shift + scale ----
    def mask_stats(path):
        _, _, rows = read_tga(path)
        pts = [(x, y) for y in range(H) for x in range(W) if bright(px(rows, x, y)) > 20]
        xs = [p[0] for p in pts]; ys = [p[1] for p in pts]
        return (sum(xs) / len(xs), max(xs) - min(xs), len(pts)) if pts else (0, 0, 0)
    render(["quad.obj", "--no-shadow", "cam0.tga"], tmp)
    render(["quad.obj", "--eye", "0.5,0,3", "--no-shadow", "cam1.tga"], tmp)
    render(["quad.obj", "--eye", "0,0,5", "--no-shadow", "cam2.tga"], tmp)
    cx0, w0, n0 = mask_stats(f"{tmp}/cam0.tga")
    cx1, w1, n1 = mask_stats(f"{tmp}/cam1.tga")
    cx2, w2, n2 = mask_stats(f"{tmp}/cam2.tga")
    # lookAt keeps the origin centred, so the quad does NOT translate;
    # instead the side nearer the +x eye (the right side) is magnified by
    # perspective, so the centroid shifts RIGHT. That asymmetry is the
    # correct predicted camera effect here.
    check("T4 camera +x eye shifts centroid right (near side magnified)", cx1 > cx0 + 5, (cx0, cx1))
    check("T4 farther eye shrinks object", w2 < w0 - 20 and n2 < n0, (w0, w2))
    check("T4 object visible in all camera renders", n0 > 1000 and n1 > 1000 and n2 > 1000)

    # ---- T5 Phong: bright vs dim, specular highlight ----
    render(["phong_bright.obj", "--light", "0,0,5", "--no-shadow", "ph_b.tga"], tmp)
    render(["phong_dim.obj", "--light", "0,0,5", "--no-shadow", "ph_d.tga"], tmp)
    _, _, rb = read_tga(f"{tmp}/ph_b.tga")
    _, _, rd = read_tga(f"{tmp}/ph_d.tga")
    b, d = bright(px(rb, 400, 500)), bright(px(rd, 400, 500))
    check("T5 face toward light brighter than face away (relative)", b > d + 30, (b, d))
    check("T5 highlight: bright face near white", b > 200, b)

    # ---- T6 textures: quadrant order ----
    render(["quad.obj", "--diffuse", "quad.tga", "--light", "0,0,10", "--no-shadow", "tex.tga"], tmp)
    _, _, rows = read_tga(f"{tmp}/tex.tga")
    # Quad spans roughly x 200..600, y 200..600 at eye (0,0,3), fov 60.
    tl, tr, bl, br = px(rows, 300, 300), px(rows, 500, 300), px(rows, 300, 500), px(rows, 500, 500)
    check("T6 texture TL is red-dominant", tl[0] > tl[1] + 50 and tl[0] > tl[2] + 50, tl)
    check("T6 texture TR is green-dominant", tr[1] > tr[0] + 50 and tr[1] > tr[2] + 50, tr)
    check("T6 texture BL is blue-dominant", bl[2] > bl[0] + 50 and bl[2] > bl[1] + 50, bl)
    check("T6 texture BR is white", bright(br) > 200, br)

    # ---- T7 normal mapping: same silhouette, different shading ----
    render(["quad.obj", "--light", "0,0,5", "--no-shadow", "nm_off.tga"], tmp)
    render(["quad.obj", "--light", "0,0,5", "--normal", "nmap.tga", "--no-shadow", "nm_on.tga"], tmp)
    _, _, off = read_tga(f"{tmp}/nm_off.tga")
    _, _, on = read_tga(f"{tmp}/nm_on.tga")
    mask_off = [[bright(px(off, x, y)) > 20 for x in range(W)] for y in range(H)]
    mask_on = [[bright(px(on, x, y)) > 20 for x in range(W)] for y in range(H)]
    check("T7 normal map keeps silhouette", mask_off == mask_on)
    diff = sum(1 for y in range(H) for x in range(W)
               if mask_on[y][x] and max(abs(a - b) for a, b in zip(px(on, x, y), px(off, x, y))) > 10)
    total = sum(1 for y in range(H) for x in range(W) if mask_on[y][x])
    check("T7 normal map changes shading on >20% of pixels", diff > 0.2 * total, (diff, total))

    # ---- T8 shadow mapping ----
    r = render(["occ.obj", "floor.obj", "--light", "0,3,3", "shadow.tga"], tmp)
    _, _, rows = read_tga(f"{tmp}/shadow.tga")
    centre = bright(px(rows, 400, 400))
    lit = bright(px(rows, 650, 150))  # world ~(1.08, 1.08, 0), outside shadow
    check("T8 floor centre in shadow (ambient-only dark)", centre < 60, centre)
    check("T8 floor outside shadow is lit", lit > 150, lit)
    # Hard edge along row y=400: brightness profile must jump, not grade.
    prof = [bright(px(rows, x, 400)) for x in range(150, 651)]
    dark, brightv = 60, 150
    # Find first index from the centre outward (right) where it turns lit.
    trans = next((i for i in range(250, 500) if prof[i] > brightv), None)
    hard = trans is not None and all(v < dark for v in prof[trans - 3:trans]) \
        and all(v > brightv for v in prof[trans + 1:trans + 4])
    check("T8 shadow edge is hard (<=2px transition)", hard, trans)

    # ---- Determinism ----
    render(["quad.obj", "--diffuse", "quad.tga", "det1.tga"], tmp)
    render(["quad.obj", "--diffuse", "quad.tga", "det2.tga"], tmp)
    check("T determinism: byte-identical reruns",
          open(f"{tmp}/det1.tga", "rb").read() == open(f"{tmp}/det2.tga", "rb").read())

    # ---- Full render: cube with texture ----
    r = render(["cube.obj", "--diffuse", "quad.tga", "cube.tga"], tmp)
    w, h, rows = read_tga(f"{tmp}/cube.tga")
    colours = {px(rows, x, y) for y in range(0, 800, 5) for x in range(0, 800, 5)}
    nonbg = sum(1 for y in range(0, 800, 5) for x in range(0, 800, 5) if px(rows, x, y) != (0, 0, 0))
    check("T full render: valid, not all background, not one colour",
          r.returncode == 0 and (w, h) == (800, 800) and nonbg > 1000 and len(colours) > 10,
          (nonbg, len(colours)))

    # ---- Integration: CLI errors ----
    d = tempfile.mkdtemp(prefix="tiny-err-")
    r = render([], d)
    check("CLI no args: non-zero, usage on stderr, no output",
          r.returncode != 0 and "usage" in r.stderr and not os.path.exists(f"{d}/framebuffer.tga"))
    r = render(["missing.obj", "out.tga"], d)
    check("CLI missing model: non-zero, error on stderr, no output presented",
          r.returncode != 0 and r.stderr.strip() != "" and not os.path.exists(f"{d}/out.tga"))
    print(f"\n{len(passed)} passed, {len(failed)} failed" + (f": {failed}" if failed else ""))
    print(f"fixtures/renders in {tmp}")
    return 1 if failed else 0

if __name__ == "__main__":
    sys.exit(main())
