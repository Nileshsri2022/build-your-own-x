"""Numpy helpers for coordinate spaces (T1).

Column-vector convention: a point p is transformed by M @ p.
A node's world transform is translation @ scaling (separate
matrices, so scaling never moves a node and translating never
rescales it). No OpenGL calls live here, so this is headless.
"""
import numpy as np

IDENTITY = np.eye(4)


def translation_matrix(x, y, z):
    m = np.eye(4)
    m[:3, 3] = (x, y, z)
    return m


def scaling_matrix(sx, sy=None, sz=None):
    if sy is None:
        sy = sx
    if sz is None:
        sz = sx
    m = np.eye(4)
    m[0, 0], m[1, 1], m[2, 2] = sx, sy, sz
    return m


def perspective_matrix(fov_degrees, aspect, near, far):
    """Standard OpenGL-style perspective matrix (eye -> clip)."""
    f = 1.0 / np.tan(np.radians(fov_degrees) / 2.0)
    m = np.zeros((4, 4))
    m[0, 0] = f / aspect
    m[1, 1] = f
    m[2, 2] = (far + near) / (near - far)
    m[2, 3] = 2.0 * far * near / (near - far)
    m[3, 2] = -1.0
    return m


def apply_matrix(matrix, point):
    """Transform a 3-vector point, returning a 3-vector."""
    p = np.append(np.asarray(point, dtype=float), 1.0)
    out = matrix @ p
    return out[:3] / out[3] if out[3] != 0 else out[:3]


def transform_aabb(matrix, aabb_min, aabb_max):
    """Transform an AABB by a matrix; result is the enclosing AABB."""
    corners = np.array([[x, y, z] for x in (aabb_min[0], aabb_max[0])
                        for y in (aabb_min[1], aabb_max[1])
                        for z in (aabb_min[2], aabb_max[2])])
    moved = np.array([apply_matrix(matrix, c) for c in corners])
    return moved.min(axis=0), moved.max(axis=0)


class Ray:
    """A ray: origin + t * direction, direction normalised."""

    def __init__(self, origin, direction):
        self.origin = np.asarray(origin, dtype=float)
        d = np.asarray(direction, dtype=float)
        n = np.linalg.norm(d)
        if n == 0:
            raise ValueError("ray direction must be non-zero")
        self.direction = d / n

    def point_at(self, t):
        return self.origin + self.direction * t


def ray_hits_aabb(ray, aabb_min, aabb_max):
    """Slab test. Returns the smallest non-negative t, or None.

    A ray starting inside the box hits at t = 0.
    """
    t_min, t_max = 0.0, float("inf")
    for origin, direction, lo, hi in zip(ray.origin, ray.direction,
                                         aabb_min, aabb_max):
        if abs(direction) < 1e-12:
            if origin < lo or origin > hi:
                return None
        else:
            t1 = (lo - origin) / direction
            t2 = (hi - origin) / direction
            t_min = max(t_min, min(t1, t2))
            t_max = min(t_max, max(t1, t2))
            if t_min > t_max:
                return None
    return t_min


def build_pick_ray(mouse_x, mouse_y, width, height, modelview, projection):
    """Pick ray from a mouse position (DESIGN: unproject at 0.001/0.999).

    Screen y grows downwards; NDC y grows upwards, hence the flip.
    """
    ndc_x = 2.0 * mouse_x / width - 1.0
    ndc_y = 1.0 - 2.0 * mouse_y / height
    inv = np.linalg.inv(projection @ modelview)
    points = []
    for depth in (0.001, 0.999):
        clip = np.array([ndc_x, ndc_y, 2.0 * depth - 1.0, 1.0])
        world = inv @ clip
        points.append(world[:3] / world[3])
    return Ray(points[0], points[1] - points[0])
