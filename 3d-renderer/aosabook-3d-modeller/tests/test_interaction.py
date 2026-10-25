import unittest
import numpy as np
from modeller.interaction import Interaction
from modeller.matrices import Ray, translation_matrix, IDENTITY
from modeller.nodes import Cube, Sphere, SnowFigure
from modeller.scene import Scene


class FakeTrackball:
    matrix = translation_matrix(1, 0, 0)


def ray_down_z(origin=(0, 0, 0)):
    return Ray(origin, [0, 0, -1])


class TestInteraction(unittest.TestCase):
    def setUp(self):
        self.scene = Scene()
        self.inter = Interaction(self.scene)
        self.inter.register_callback("pick", self.inter.on_pick)
        self.inter.register_callback("move", self.inter.on_move)
        self.inter.register_callback("place", self.inter.on_place)
        self.inter.register_callback("rotate_colour", self.inter.on_rotate_colour)
        self.inter.register_callback("scale", self.inter.on_scale)

    def test_register_and_trigger(self):
        seen = []
        self.inter.register_callback("custom", seen.append)
        self.inter.trigger("custom", 42)
        self.assertEqual(seen, [42])

    def test_unregistered_trigger_is_noop(self):
        self.inter.trigger("no_such_event")  # must not crash

    def test_no_selection_safety(self):
        self.inter.trigger("scale", 1.1)
        self.inter.trigger("rotate_colour")
        self.inter.trigger("move", ray_down_z())
        self.assertIsNone(self.scene.selected_node)

    def test_place_each_kind_adds_one_node_at_place_depth(self):
        for kind, cls in (("sphere", Sphere), ("cube", Cube),
                          ("figure", SnowFigure)):
            before = len(self.scene.node_list)
            self.inter.trigger("place", kind, ray_down_z())
            self.assertEqual(len(self.scene.node_list), before + 1)
            node = self.scene.node_list[-1]
            self.assertIsInstance(node, cls)
            np.testing.assert_allclose(node.translation[:3, 3], [0, 0, -15])
            self.assertEqual(node.place_depth, 15.0)

    def test_move_selected_to_ray_at_stored_depth(self):
        cube = Cube()
        self.scene.add_node(cube)
        self.scene.selected_node = cube
        self.inter.trigger("move", Ray([3, 4, 0], [0, 0, -1]))
        np.testing.assert_allclose(cube.translation[:3, 3], [3, 4, -15])

    def test_keyboard_mapping(self):
        self.inter.on_key("s", ray_down_z())
        self.assertIsInstance(self.scene.node_list[-1], Sphere)
        self.inter.on_key("c", ray_down_z())
        self.assertIsInstance(self.scene.node_list[-1], Cube)
        node = self.scene.selected_node
        self.inter.on_key("up")
        self.assertAlmostEqual(node.scaling[0, 0], 1.1)
        self.inter.on_key("down")
        self.assertAlmostEqual(node.scaling[0, 0], 0.99)
        colour = node.colour_index
        self.inter.on_key("left")
        self.assertEqual(node.colour_index, (colour + 1) % 6)

    def test_trackball_matrix_enters_modelview(self):
        inter = Interaction(self.scene, trackball=FakeTrackball())
        np.testing.assert_allclose(inter.modelview_matrix(),
                                   translation_matrix(1, 0, 0))

    def test_missing_glumpy_raises_clear_error(self):
        from modeller.trackball import GlumpyTrackball
        with self.assertRaises(RuntimeError) as ctx:
            GlumpyTrackball()
        self.assertIn("Glumpy", str(ctx.exception))


if __name__ == "__main__":
    unittest.main()
