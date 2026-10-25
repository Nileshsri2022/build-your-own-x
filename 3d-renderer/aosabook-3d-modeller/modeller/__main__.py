"""Entry point: build Viewer, enter main loop (window only)."""
import sys
from .viewer import Viewer


def main():
    viewer = Viewer()
    try:
        viewer.main_loop()
    except RuntimeError as exc:
        print(f"3D Modeller cannot start: {exc}", file=sys.stderr)
        return 1
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
