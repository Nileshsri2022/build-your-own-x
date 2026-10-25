"""Viewer (T3): window, OpenGL state, render pass, main loop.

All projection/camera numbers are exactly SPEC section 3 and are
plain constants / pure functions, so they are testable headlessly.
Anything needing a real OpenGL context imports it lazily inside
init_opengl()/main_loop() and fails with a message naming the
missing piece (PyOpenGL, GLUT, or glumpy) instead of a bare import
traceback at module load.
"""
import numpy as np
from .matrices import IDENTITY, perspective_matrix, translation_matrix
from .scene import build_sample_scene
from .interaction import Interaction
from .trackball import GlumpyTrackball

WIDTH, HEIGHT = 640, 480
TITLE = "3D Modeller"
FOV_DEGREES = 70
NEAR, FAR = 0.1, 1000.0
CAMERA = (0.0, 0.0, -15.0)
CLEAR_COLOUR = (0.4, 0.4, 0.4, 0.0)
LIGHT_POSITION = (0.0, 0.0, 1.0, 0.0)   # directional: 4th component 0
LIGHT_SPOT_DIRECTION = (0.0, 0.0, -1.0)


def projection_matrix(width=WIDTH, height=HEIGHT):
    return perspective_matrix(FOV_DEGREES, width / height, NEAR, FAR)


def camera_matrix():
    return translation_matrix(*CAMERA)


class Viewer:
    """Owns scene + interaction; render() takes a backend so the pass
    (projection, ModelView, scene, grid, flush) is testable with a stub
    backend, while the real GL backend is only built in main_loop()."""

    def __init__(self):
        self.scene = build_sample_scene()
        self.interaction = Interaction(self.scene)
        self.modelview = IDENTITY
        self.modelview_inverse = IDENTITY
        self._register_callbacks()

    def _register_callbacks(self):
        i = self.interaction
        i.register_callback("pick", i.on_pick)
        i.register_callback("move", i.on_move)
        i.register_callback("place", i.on_place)
        i.register_callback("rotate_colour", i.on_rotate_colour)
        i.register_callback("scale", i.on_scale)

    def render(self, backend):
        """Render pass: projection, ModelView from interaction state,
        scene with lighting on, grid with lighting off, flush."""
        projection = projection_matrix()
        self.modelview = self.interaction.modelview_matrix() @ camera_matrix()
        self.modelview_inverse = np.linalg.inv(self.modelview)
        self.interaction.modelview_inverse = self.modelview_inverse
        backend.set_projection(projection)
        backend.set_modelview(self.modelview)
        backend.enable_lighting(True)
        backend.clear()
        self.scene.render(backend)
        backend.enable_lighting(False)
        backend.draw_grid()
        backend.flush()

    # -- window-dependent part (needs a real GL context) ------------------
    def init_opengl(self):
        try:
            import OpenGL.GL as gl  # noqa: F401
            from OpenGL.GLUT import glutInit  # noqa: F401
        except ImportError as exc:
            raise RuntimeError(
                "PyOpenGL/GLUT is not installed; the interactive viewer "
                "cannot run here. Headless scene/picking tests do not "
                "need it.") from exc

    def main_loop(self):
        self.init_opengl()
        try:
            self.interaction.trackball = GlumpyTrackball()
        except RuntimeError as exc:
            raise RuntimeError(str(exc)) from exc
        raise RuntimeError(
            "GLUT main loop cannot run in this headless environment: "
            "no display/GL context is available here. Run on a machine "
            "with PyOpenGL, GLUT and glumpy to open the window.")
