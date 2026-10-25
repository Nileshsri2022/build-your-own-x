#!/bin/sh
# Integration test (TEST_PLAN.md): real Main process, PNG validity,
# centre/corner pixels, and bad-argument behaviour.
# Usage: integration.sh <classes-dir> <work-dir>
set -u
case "$1" in
  /*) CLASSES="$1" ;;
  *)  CLASSES="$PWD/$1" ;;
esac
WORK="$2"
JAVA="${JAVA:-java}"
mkdir -p "$WORK"
cd "$WORK" || exit 1

"$JAVA" -cp "$CLASSES" com.renderer.Main out.png 100 100 4
rc=$?
[ "$rc" -eq 0 ] || { echo "FAIL: Main 100x100x4 exit=$rc"; exit 1; }
[ -f out.png ] || { echo "FAIL: out.png missing"; exit 1; }

# No arguments: non-zero exit, usage on stderr, no file created.
rm -f out2.png
"$JAVA" -cp "$CLASSES" com.renderer.Main 2> err.txt
rc=$?
[ "$rc" -ne 0 ] || { echo "FAIL: no-arg Main should exit non-zero"; exit 1; }
grep -q usage err.txt || { echo "FAIL: no-arg Main should print usage"; exit 1; }
[ ! -f out2.png ] || { echo "FAIL: no-arg Main created a file"; exit 1; }

# Bad extension: non-zero exit.
"$JAVA" -cp "$CLASSES" com.renderer.Main out.txt 2> err2.txt
rc=$?
[ "$rc" -ne 0 ] || { echo "FAIL: out.txt Main should exit non-zero"; exit 1; }
[ ! -f out.txt ] || { echo "FAIL: out.txt was created"; exit 1; }

# Non-positive width: non-zero exit, no file.
"$JAVA" -cp "$CLASSES" com.renderer.Main bad.png 0 100 1 2> err3.txt
rc=$?
[ "$rc" -ne 0 ] || { echo "FAIL: width 0 should exit non-zero"; exit 1; }
[ ! -f bad.png ] || { echo "FAIL: bad.png was created"; exit 1; }

echo "INTEGRATION SHELL PASS (PNG pixel checks run by PixelCheck)"
