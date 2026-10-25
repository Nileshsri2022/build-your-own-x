#!/bin/bash
# Integration test (TEST_PLAN §2): build, render default scene 100x100,
# check PPM dims, centre vs background, determinism, CLI failures.
set -u
cd "$(dirname "$0")/.." || exit 1
make -s pbrt || exit 1
TMP=$(mktemp -d)
trap 'rm -rf "$TMP"' EXIT

./pbrt "$TMP/a.ppm" 100 100 4 42 || { echo "FAIL: render exit"; exit 1; }
./pbrt "$TMP/b.ppm" 100 100 4 42 || { echo "FAIL: second render exit"; exit 1; }
cmp -s "$TMP/a.ppm" "$TMP/b.ppm" || { echo "FAIL: same seed renders differ"; exit 1; }
head -3 "$TMP/a.ppm" | tr '\n' ' ' > "$TMP/hdr"
grep -q "^P3 100 100 255 $" "$TMP/hdr" || { echo "FAIL: header: $(cat "$TMP/hdr")"; exit 1; }
# Centre pixel (row 50, col 50) must not be background (13,17,25);
# top-left corner pixel must be background.
CENTRE=$(sed -n '5054p' "$TMP/a.ppm")   # header 3 lines + 50*100 + 50 + 1
CORNER=$(sed -n '4p' "$TMP/a.ppm")
echo "centre=$CENTRE corner=$CORNER"
[ "$CENTRE" != "12 17 25" ] && [ "$CENTRE" != "13 17 25" ] || { echo "FAIL: centre is background"; exit 1; }
[ "$CORNER" = "12 17 25" ] || { echo "FAIL: corner not background: $CORNER"; exit 1; }
# No arguments: non-zero, usage on stderr, no file created.
if ./pbrt 2>"$TMP/err"; then echo "FAIL: no-args exit 0"; exit 1; fi
grep -q usage "$TMP/err" || { echo "FAIL: no usage on stderr"; exit 1; }
# Unwritable output path: non-zero, error, no success claimed.
if ./pbrt "$TMP/nonexistent-dir/out.ppm" 10 10 1 1 2>"$TMP/err2"; then echo "FAIL: bad path exit 0"; exit 1; fi
grep -q error "$TMP/err2" || { echo "FAIL: no error message"; exit 1; }
echo "INTEGRATION PASSED"
