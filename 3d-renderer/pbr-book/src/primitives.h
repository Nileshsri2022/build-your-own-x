// Ch. 7 — Primitives and BVH. A Primitive is a shape + its BSDF.
// BVH: median split on the longest centroid axis, leaf size 4.
// Traversal must return the same nearest hit as brute force (tested).
#pragma once
#include <vector>
#include <memory>
#include <algorithm>
#include "shapes.h"
#include "bsdf.h"

namespace pbr {

struct Primitive {
    std::shared_ptr<Shape> shape;
    std::shared_ptr<BSDF> bsdf;
    Bounds bounds() const { return shape->bounds(); }
};

struct SceneHit {
    Hit hit; const Primitive* prim = nullptr;
};

class BVH {
public:
    explicit BVH(std::vector<Primitive> prims) : prims_(std::move(prims)) {
        std::vector<int> idx(prims_.size());
        for (size_t i = 0; i < idx.size(); ++i) idx[i] = (int)i;
        if (!idx.empty()) build(idx, 0);
    }
    bool intersect(const Ray& ray, double tMin, SceneHit& out) const {
        if (nodes_.empty()) return false;
        bool found = false; double best = ray.tMax;
        std::vector<int> stack{0};
        while (!stack.empty()) {
            int ni = stack.back(); stack.pop_back();
            const Node& node = nodes_[ni];
            if (!node.bounds.hit(ray, tMin, best)) continue;
            if (node.leaf) {
                for (int pi : node.prims) {
                    Hit h; Ray r = ray; r.tMax = best;
                    if (prims_[pi].shape->intersect(r, tMin, h) && h.t < best) {
                        best = h.t; out.hit = h; out.prim = &prims_[pi]; found = true;
                    }
                }
            } else {
                stack.push_back(node.left); stack.push_back(node.right);
            }
        }
        return found;
    }
    size_t nodeCount() const { return nodes_.size(); }
private:
    struct Node { Bounds bounds; int left = -1, right = -1; std::vector<int> prims; bool leaf = false; };
    std::vector<Primitive> prims_;
    std::vector<Node> nodes_;
    int build(std::vector<int>& idx, int depth) {
        int me = (int)nodes_.size(); nodes_.push_back(Node{});
        Bounds b; for (int i : idx) b.grow(prims_[i].bounds());
        nodes_[me].bounds = b;
        if (idx.size() <= 4 || depth > 32) {
            nodes_[me].leaf = true; nodes_[me].prims = idx; return me;
        }
        Bounds cb; for (int i : idx) cb.grow(prims_[i].bounds().centroid());
        int axis = cb.longestAxis();
        auto coord = [&](int i) {
            Vec3 c = prims_[i].bounds().centroid();
            return axis == 0 ? c.x : (axis == 1 ? c.y : c.z);
        };
        std::sort(idx.begin(), idx.end(), [&](int a, int c) { return coord(a) < coord(c); });
        size_t mid = idx.size() / 2;
        std::vector<int> left(idx.begin(), idx.begin() + (long)mid), right(idx.begin() + (long)mid, idx.end());
        nodes_[me].left = build(left, depth + 1);
        nodes_[me].right = build(right, depth + 1);
        return me;
    }
};

// Brute-force aggregate, used by tests to verify the BVH.
inline bool bruteIntersect(const std::vector<Primitive>& prims, const Ray& ray, double tMin, SceneHit& out) {
    bool found = false; double best = ray.tMax;
    for (const auto& p : prims) {
        Hit h; Ray r = ray; r.tMax = best;
        if (p.shape->intersect(r, tMin, h) && h.t < best) {
            best = h.t; out.hit = h; out.prim = &p; found = true;
        }
    }
    return found;
}

}  // namespace pbr
