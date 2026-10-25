import unittest
import numpy as np
from modeller.matrices import Ray, build_pick_ray, IDENTITY
from modeller.nodes import Cube, Sphere, SnowFigure
from modeller.scene import Scene, build_sample_scene
from modeller.viewer import projection_matrix


class TestPicking(unittest.TestCase):
    def test_centre_ray_selects_cube(self):
        s = Scene()
        cube = Cube()
        cube.translate(0, 0, -5)
        s.add_node(cube)
        hit = s.pick(Ray([0, 0, 0], [0, 0, -1]))
        self.assertIs(hit, cube)

    def test_closer_of_two_overlapping_boxes_wins(self):
        s = Scene()
        far, near = Cube(), Cube()
        far.translate(0, 0, -8)
        near.translate(0, 0, -4)
        s.add_node(far)
        s.add_node(near)
        self.assertIs(s.pick(Ray([0, 0, 0], [0, 0, -1])), near)

    def test_miss_selects_nothing(self):
        s = Scene()
        cube = Cube()
        cube.translate(0, 0, -5)
        s.add_node(cube)
        self.assertIsNone(s.pick(Ray([10, 10, 0], [0, 0, -1])))
        self.assertIsNone(s.selected_node)

    def test_aabb_corner_ray_selects_sphere_it_misses(self):
        """Documented approximation: ray through the box corner region
        (0.95, 0.95) is inside the sphere's AABB but 1.34 units from the
        sphere centre, so it misses the sphere itself — yet selects it."""
        s = Scene()
        sphere = Sphere()
        s.add_node(sphere)
        ray = Ray([0.95, 0.95, -5], [0, 0, 1])
        self.assertGreater(np.linalg.norm([0.95, 0.95]), 1.0)  # misses sphere
        self.assertIs(s.pick(ray), sphere)

    def test_parent_transform_applied_for_figure(self):
        s = build_sample_scene()
        figure = s.node_list[2]
        # Middle child sphere world centre: figure (-2,0,-2) + (0,1.5,0).
        hit = s.pick(Ray([-2, 1.5, -10], [0, 0, 1]))
        self.assertIs(hit, figure)

    def test_pick_ray_from_mouse_centre_points_into_scene(self):
        ray = build_pick_ray(320, 240, 640, 480, IDENTITY,
                             projection_matrix())
        np.testing.assert_allclose(ray.direction, [0, 0, -1], atol=1e-9)
        # Near point (depth 0.001) is in front of far point (0.999).
        self.assertLess(ray.origin[2], 0)


if __name__ == "__main__":
    unittest.main()
