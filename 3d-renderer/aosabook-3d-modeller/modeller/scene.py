"""Scene (T2): node list in insertion order, selected node, place depth."""
from .matrices import IDENTITY
from .nodes import Cube, Sphere, SnowFigure
from .picking import pick

PLACE_DEPTH = 15.0


class Scene:
    def __init__(self):
        self.node_list = []
        self.selected_node = None

    def add_node(self, node):
        self.node_list.append(node)

    def render(self, backend):
        for node in self.node_list:
            node.render(backend, selected=(node is self.selected_node))

    def pick(self, ray):
        self.selected_node = pick(self.node_list, ray)
        return self.selected_node


def build_sample_scene():
    """SPEC section 4: cube (2,0,2), sphere (-2,0,2), snow figure
    (-2,0,-2), each with its own colour index."""
    scene = Scene()
    cube, sphere, figure = Cube(0), Sphere(1), SnowFigure(2)
    cube.translate(2, 0, 2)
    sphere.translate(-2, 0, 2)
    figure.translate(-2, 0, -2)
    for node in (cube, sphere, figure):
        scene.add_node(node)
    return scene
