"""Picking (T6): the only ray in this project. AABB approximation by
design — a ray through a sphere's box corner selects the sphere even
though it misses the sphere itself (see SPEC section 6). Accurate
ray-object intersection is a listed future extension, NOT built here.
"""
from .matrices import IDENTITY, ray_hits_aabb


def closest_hit(node, ray, parent_matrix=IDENTITY):
    """Smallest ray distance hitting this node or any descendant.

    For a hierarchical node, descendants are tested under the parent
    transform; the hit belongs to the top-level node, not the child.
    Returns None when nothing under this node is hit.
    """
    from .nodes import HierarchicalNode
    if isinstance(node, HierarchicalNode):
        matrix = parent_matrix @ node.local_matrix()
        hits = [closest_hit(c, ray, matrix) for c in node.children]
        hits = [h for h in hits if h is not None]
        return min(hits) if hits else None
    lo, hi = node.world_aabb(parent_matrix)
    return ray_hits_aabb(ray, lo, hi)


def pick(nodes, ray):
    """Closest-hit top-level node, or None when the ray hits nothing."""
    best, best_t = None, None
    for node in nodes:
        t = closest_hit(node, ray)
        if t is not None and (best_t is None or t < best_t):
            best, best_t = node, t
    return best
