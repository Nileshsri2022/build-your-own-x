import unittest
import numpy as np
from modeller.matrices import translation_matrix, scaling_matrix, IDENTITY
from modeller.nodes import Cube, Sphere, SnowFigure, COLOURS
from tests.stub_backend import StubBackend


class TestTransforms(unittest.TestCase):
    def test_translate_does_not_change_scale(self):
        n = Cube()
        n.translate(2, 0, 2)
        np.testing.assert_allclose(n.scaling, IDENTITY)
        np.testing.assert_allclose(n.translation, translation_matrix(2, 0, 2))

    def test_scale_does_not_move(self):
        n = Cube()
        n.scale(1.1)
        np.testing.assert_allclose(n.translation, IDENTITY)
        np.testing.assert_allclose(n.scaling, scaling_matrix(1.1))

    def test_scale_is_multiplicative_not_round_trip(self):
        n = Cube()
        n.scale(1.1)
        n.scale(0.9)
        self.assertAlmostEqual(n.scaling[0, 0], 1.1 * 0.9)  # 0.99, not 1.0

    def test_colour_wraps(self):
        n = Cube(len(COLOURS) - 1)
        n.rotate_colour()
        self.assertEqual(n.colour_index, 0)
        self.assertEqual(n.colour, COLOURS[0])

    def test_snow_figure_is_one_unit_with_relative_children(self):
        fig = SnowFigure()
        fig.translate(-2, 0, -2)
        fig.scale(2.0)
        # Children keep relative offsets: middle is 1.5 above bottom.
        bottom, middle = fig.children[0], fig.children[1]
        np.testing.assert_allclose(
            middle.translation[:3, 3] - bottom.translation[:3, 3],
            [0, 1.5, 0])

    def test_hierarchical_render_restores_matrix(self):
        fig = SnowFigure()
        fig.translate(1, 0, 0)
        sibling = Cube()
        backend = StubBackend()
        fig.render(backend)
        sibling.render(backend)
        self.assertEqual(len(backend.stack), 1)  # no leaked transform
        np.testing.assert_allclose(backend.current_matrix(), IDENTITY)
        # Figure drew 3 spheres, then the sibling cube, in order.
        self.assertEqual(backend.draws, ["sphere", "sphere", "sphere", "cube"])


if __name__ == "__main__":
    unittest.main()
