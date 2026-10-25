"""Scene-graph nodes (T2). No OpenGL calls here: render() talks to a
backend object (push/pop/mult/draw), so a stub backend stands in for
OpenGL in headless tests and the real GL backend in the viewer.
"""
import numpy as np
from .matrices import IDENTITY, translation_matrix, scaling_matrix, transform_aabb

# Code decision (DESIGN): the chapter cycles a colour list; its exact
# entries are not a spec requirement, only the wrap behaviour is.
COLOURS = [(1.0, 0.0, 0.0), (0.0, 1.0, 0.0), (0.0, 0.0, 1.0),
           (1.0, 1.0, 0.0), (1.0, 0.0, 1.0), (0.0, 1.0, 1.0)]
SCALE_UP, SCALE_DOWN = 1.1, 0.9


class Node:
    """Base node: separate translation and scaling matrices, a colour
    index, and a local AABB. Subclasses set local_aabb."""

    local_aabb = (np.array([-0.5, -0.5, -0.5]), np.array([0.5, 0.5, 0.5]))

    def __init__(self, colour_index=0):
        self.translation = np.eye(4)
        self.scaling = np.eye(4)
        self.colour_index = colour_index
        self.place_depth = 15.0  # stored depth used when moving this node

    @property
    def colour(self):
        return COLOURS[self.colour_index % len(COLOURS)]

    def translate(self, x, y, z):
        self.translation = translation_matrix(x, y, z)

    def set_translation(self, point):
        self.translation = translation_matrix(*point)

    def scale(self, factor):
        self.scaling = self.scaling @ scaling_matrix(factor)

    def rotate_colour(self):
        self.colour_index = (self.colour_index + 1) % len(COLOURS)

    def local_matrix(self):
        return self.translation @ self.scaling

    def world_aabb(self, parent_matrix=IDENTITY):
        lo, hi = self.local_aabb
        return transform_aabb(parent_matrix @ self.local_matrix(), lo, hi)

    def render(self, backend, parent_matrix=IDENTITY, selected=False):
        backend.push_matrix()
        try:
            backend.mult_matrix(self.translation)
            backend.mult_matrix(self.scaling)
            self._render_content(backend, selected)
        finally:
            backend.pop_matrix()

    def _render_content(self, backend, selected):
        pass  # base node has no content of its own


class Primitive(Node):
    """A node rendered via one OpenGL call list (name only, headless)."""

    call_list_name = "primitive"

    def _render_content(self, backend, selected):
        backend.set_emission(selected)  # selected node glows
        backend.draw_call_list(self.call_list_name)


class Cube(Primitive):
    call_list_name = "cube"
    local_aabb = (np.array([-0.5, -0.5, -0.5]), np.array([0.5, 0.5, 0.5]))


class Sphere(Primitive):
    call_list_name = "sphere"
    local_aabb = (np.array([-1.0, -1.0, -1.0]), np.array([1.0, 1.0, 1.0]))


class HierarchicalNode(Node):
    """Composite: children render under this node's transform."""

    def __init__(self, colour_index=0, children=()):
        super().__init__(colour_index)
        self.children = list(children)

    def add_child(self, child):
        self.children.append(child)

    def _render_content(self, backend, selected):
        for child in self.children:
            child.render(backend, selected=selected)

    def world_aabb(self, parent_matrix=IDENTITY):
        # Enclosing box of all children, in world space.
        matrix = parent_matrix @ self.local_matrix()
        boxes = [c.world_aabb(matrix) for c in self.children]
        if not boxes:
            return self.local_aabb
        lo = np.min([b[0] for b in boxes], axis=0)
        hi = np.max([b[1] for b in boxes], axis=0)
        return lo, hi


class SnowFigure(HierarchicalNode):
    """Three spheres grouped as one node; the scene treats it as one
    node, so selecting/moving/scaling/colouring it acts on the unit
    while children keep their relative positions."""

    def __init__(self, colour_index=0):
        bottom, middle, top = Sphere(colour_index), Sphere(colour_index), Sphere(colour_index)
        middle.translate(0.0, 1.5, 0.0)
        top.translate(0.0, 2.5, 0.0)
        super().__init__(colour_index, [bottom, middle, top])
