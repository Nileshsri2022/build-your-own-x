import unittest
import numpy as np
from modeller.matrices import IDENTITY
from modeller.nodes import Cube, Sphere, SnowFigure
from modeller.scene import Scene, build_sample_scene, PLACE_DEPTH
from modeller.viewer import (Viewer, WIDTH, HEIGHT, TITLE, FOV_DEGREES,
                             NEAR, FAR, CAMERA, CLEAR_COLOUR,
                             LIGHT_POSITION, LIGHT_SPOT_DIRECTION,
                             projection_matrix)
from tests.stub_backend import StubBackend


class TestScene(unittest.TestCase):
    def test_insertion_and_render_order(self):
        s, backend = Scene(), StubBackend()
        s.add_node(Cube())
        s.add_node(Sphere())
        s.render(backend)
        self.assertEqual(backend.draws, ["cube", "sphere"])
        self.assertEqual(len(s.node_list), 2)

    def test_selection_set_and_cleared(self):
        s = Scene()
        cube = Cube()
        s.add_node(cube)
        self.assertIsNone(s.selected_node)
        s.selected_node = cube
        self.assertIs(s.selected_node, cube)
        s.selected_node = None
        self.assertIsNone(s.selected_node)

    def test_sample_scene(self):
        s = build_sample_scene()
        self.assertEqual(len(s.node_list), 3)
        cube, sphere, figure = s.node_list
        self.assertIsInstance(cube, Cube)
        self.assertIsInstance(sphere, Sphere)
        self.assertIsInstance(figure, SnowFigure)
        np.testing.assert_allclose(cube.translation[:3, 3], [2, 0, 2])
        np.testing.assert_allclose(sphere.translation[:3, 3], [-2, 0, 2])
        np.testing.assert_allclose(figure.translation[:3, 3], [-2, 0, -2])
        self.assertEqual(PLACE_DEPTH, 15.0)

    def test_empty_scene_renders_nothing_but_grid(self):
        viewer = Viewer()
        viewer.scene = Scene()  # empty scene
        backend = StubBackend()
        viewer.render(backend)
        self.assertEqual(backend.draws, [])
        self.assertIn("grid", backend.log)

    def test_viewer_constants(self):
        self.assertEqual((WIDTH, HEIGHT, TITLE), (640, 480, "3D Modeller"))
        self.assertEqual(FOV_DEGREES, 70)
        self.assertEqual((NEAR, FAR), (0.1, 1000.0))
        self.assertEqual(CAMERA, (0.0, 0.0, -15.0))
        self.assertEqual(CLEAR_COLOUR, (0.4, 0.4, 0.4, 0.0))
        self.assertEqual(LIGHT_POSITION, (0.0, 0.0, 1.0, 0.0))
        self.assertEqual(LIGHT_SPOT_DIRECTION, (0.0, 0.0, -1.0))

    def test_modelview_inverse_round_trip(self):
        viewer = Viewer()
        backend = StubBackend()
        viewer.render(backend)
        point = np.array([1.0, 2.0, 3.0])
        eye = (viewer.modelview @ np.append(point, 1.0))[:3]
        back = (viewer.modelview_inverse @ np.append(eye, 1.0))[:3]
        np.testing.assert_allclose(back, point, rtol=1e-9, atol=1e-9)

    def test_render_pass_order_lighting_grid(self):
        viewer, backend = Viewer(), StubBackend()
        viewer.render(backend)
        # Scene renders with lighting on; grid renders with it off, last.
        self.assertEqual(backend.log,
                         ["lighting_on", "clear", "lighting_off",
                          "grid", "flush"])
        # Selected node glows: no selection yet, so nothing glows.
        self.assertTrue(all(not sel for _, sel in backend.emissions))
        viewer.scene.selected_node = viewer.scene.node_list[0]
        backend2 = StubBackend()
        viewer.render(backend2)
        self.assertIn(("cube", True), backend2.emissions)

    def test_projection_matrix_shape(self):
        m = projection_matrix()
        self.assertEqual(m.shape, (4, 4))
        self.assertEqual(m[3, 2], -1.0)


if __name__ == "__main__":
    unittest.main()
