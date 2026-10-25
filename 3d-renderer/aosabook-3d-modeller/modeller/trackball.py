"""Thin wrapper around the Glumpy trackball (DESIGN: imported, NOT
reimplemented here — quaternion trackball math is the chapter's
imported dependency, not this project's code).

Constructing this without glumpy installed raises a clear error
naming the missing piece. Headless code injects a stub object with
a ``matrix`` attribute instead; it never needs this class.
"""


class GlumpyTrackball:
    def __init__(self):
        try:
            from glumpy import Trackball  # type: ignore
        except ImportError as exc:
            raise RuntimeError(
                "Glumpy trackball is not installed; the interactive "
                "viewer cannot run without it (headless scene/picking "
                "tests do not need it)") from exc
        self._trackball = Trackball()

    @property
    def matrix(self):
        return self._trackball.matrix

    def drag(self, *args, **kwargs):
        return self._trackball.drag(*args, **kwargs)
