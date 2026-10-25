"""Interaction (T4/T5): callback table + OS-event translation.

No scene logic lives in an OS callback: GLUT callbacks translate a
raw event into one application event (pick/move/place/rotate_colour/
scale) and trigger it. With no selection, scale, rotate_colour and
move do nothing — they never crash.
"""
import numpy as np
from .matrices import IDENTITY, translation_matrix
from .nodes import Cube, Sphere, SnowFigure, SCALE_UP, SCALE_DOWN
from .scene import PLACE_DEPTH

EVENT_NAMES = ("pick", "move", "place", "rotate_colour", "scale")


class Interaction:
    def __init__(self, scene, trackball=None):
        self.scene = scene
        self.callbacks = {}
        self.translation = np.zeros(3)
        self.trackball = trackball  # object with .matrix, or None
        self.modelview_inverse = IDENTITY

    def register_callback(self, name, function):
        self.callbacks[name] = function

    def trigger(self, name, *args, **kwargs):
        function = self.callbacks.get(name)
        if function is not None:
            function(*args, **kwargs)

    # -- application events ------------------------------------------------
    def on_pick(self, ray):
        self.scene.pick(ray)

    def on_move(self, ray):
        node = self.scene.selected_node
        if node is None:
            return
        camera_point = ray.point_at(node.place_depth)
        world = self.modelview_inverse @ np.append(camera_point, 1.0)
        node.set_translation(world[:3])

    def on_place(self, kind, ray):
        node = {"cube": Cube, "sphere": Sphere, "figure": SnowFigure}[kind]()
        camera_point = ray.point_at(PLACE_DEPTH)
        world = self.modelview_inverse @ np.append(camera_point, 1.0)
        node.set_translation(world[:3])
        node.place_depth = PLACE_DEPTH
        self.scene.add_node(node)
        self.scene.selected_node = node  # placed node becomes the new node

    def on_scale(self, factor):
        node = self.scene.selected_node
        if node is not None:
            node.scale(factor)

    def on_rotate_colour(self):
        node = self.scene.selected_node
        if node is not None:
            node.rotate_colour()

    # -- OS callbacks (thin translation only) ------------------------------
    def on_key(self, key, ray=None):
        """Keyboard mapping: s -> place sphere, c -> place cube,
        up/down -> scale, left/right -> rotate colour."""
        if key == "s" and ray is not None:
            self.trigger("place", "sphere", ray)
        elif key == "c" and ray is not None:
            self.trigger("place", "cube", ray)
        elif key == "up":
            self.trigger("scale", SCALE_UP)
        elif key == "down":
            self.trigger("scale", SCALE_DOWN)
        elif key in ("left", "right"):
            self.trigger("rotate_colour")

    def translate_scene(self, dx, dy, dz=0.0):
        """Drag translates in the plane; wheel moves along Z."""
        self.translation = self.translation + np.array([dx, dy, dz])

    def trackball_matrix(self):
        return IDENTITY if self.trackball is None else self.trackball.matrix

    def modelview_matrix(self):
        return translation_matrix(*self.translation) @ self.trackball_matrix()
