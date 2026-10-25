#include "renderer.h"
#include <algorithm>
#include <limits>

static const double INF_DEPTH = std::numeric_limits<double>::infinity();

void Renderer::render(const std::vector<SceneModel>& models, const Vec3& eye,
                      const Vec3& center, const Vec3& up, const Light& light, bool useShadow) {
    framebuffer = TGAImage(WIDTH, HEIGHT, TGAColor(0, 0, 0));
    zbuf_.assign(size_t(WIDTH) * size_t(HEIGHT), INF_DEPTH);
    haveShadow_ = false;

    if (useShadow) {
        // Pass 1: from the light's viewpoint, depth only.
        Mat4 lightView = lookAt(light.pos, Vec3(0, 0, 0), Vec3(0, 1, 0));
        Mat4 proj = perspective(60.0, 1.0, 0.1, 100.0);
        lightViewProj_ = proj * lightView;
        shadowZbuf_.assign(size_t(WIDTH) * size_t(HEIGHT), INF_DEPTH);
        rasterizePass(models, lightViewProj_, light.pos, light, true, shadowZbuf_, nullptr);
        haveShadow_ = true;
    }

    // Pass 2: camera view, shaded.
    Mat4 view = lookAt(eye, center, up);
    Mat4 proj = perspective(60.0, 1.0, 0.1, 100.0);
    rasterizePass(models, proj * view, eye, light, false, zbuf_, &framebuffer);
}

void Renderer::rasterizePass(const std::vector<SceneModel>& models, const Mat4& viewProj,
                             const Vec3& viewEye, const Light& light, bool depthOnly,
                             std::vector<double>& depthBuf, TGAImage* outFb) {
    // View matrix alone (for view-space depth): recover by re-deriving is
    // not possible from viewProj, so depth uses clip-space w instead:
    // w = -view_z for this projection, i.e. eye-space distance. Nearer
    // fragments have smaller w, which orders exactly like view depth.
    for (const SceneModel& sm : models) {
        const Model& model = *sm.model;
        for (const Face& face : model.faces) {
            Vec3 world[3], scr[3];
            double clipW[3];
            Vec2 screen[3];
            bool behind = false;
            for (int i = 0; i < 3; i++) {
                world[i] = model.verts[face.v[i]];
                double c[4];
                transformPoint(viewProj, world[i], c);
                if (c[3] <= 0) { behind = true; break; }  // behind the camera: skip triangle
                clipW[i] = c[3];
                double ndcX = c[0] / c[3], ndcY = c[1] / c[3];
                screen[i] = {(ndcX + 1.0) * 0.5 * WIDTH, (1.0 - ndcY) * 0.5 * HEIGHT};
            }
            if (behind) continue;

            // Back-face culling: screen y grows downward, so a triangle
            // facing the camera has negative signed area; cull >= 0.
            double area2 = signedArea2(screen[0], screen[1], screen[2]);
            if (area2 >= 0) continue;

            // Per-vertex normals: from the .obj if present, else face normal.
            Vec3 faceN = normalized(cross(world[1] - world[0], world[2] - world[0]));
            Vec3 vnorm[3];
            for (int i = 0; i < 3; i++)
                vnorm[i] = face.hasNormal ? normalized(model.normals[face.vn[i]]) : faceN;

            Vec2 uv[3] = {{0, 0}, {0, 0}, {0, 0}};
            if (face.hasUV)
                for (int i = 0; i < 3; i++) uv[i] = model.uvs[face.vt[i]];

            Vec3 T, B, N;
            if (sm.normalMap && face.hasUV)
                computeTBN(world[0], world[1], world[2], uv[0], uv[1], uv[2], T, B, N);

            double minX = std::min({screen[0].x, screen[1].x, screen[2].x});
            double maxX = std::max({screen[0].x, screen[1].x, screen[2].x});
            double minY = std::min({screen[0].y, screen[1].y, screen[2].y});
            double maxY = std::max({screen[0].y, screen[1].y, screen[2].y});
            int x0 = std::max(0, (int)std::floor(minX)), x1 = std::min(WIDTH - 1, (int)std::ceil(maxX));
            int y0 = std::max(0, (int)std::floor(minY)), y1 = std::min(HEIGHT - 1, (int)std::ceil(maxY));

            for (int y = y0; y <= y1; y++) {
                for (int x = x0; x <= x1; x++) {
                    double w[3];
                    if (!barycentric(screen[0], screen[1], screen[2], {(double)x + 0.5, (double)y + 0.5}, w)) continue;
                    if (w[0] < -1e-9 || w[1] < -1e-9 || w[2] < -1e-9) continue;
                    double depth = w[0] * clipW[0] + w[1] * clipW[1] + w[2] * clipW[2];
                    size_t idx = size_t(y) * size_t(WIDTH) + size_t(x);
                    if (depth >= depthBuf[idx]) continue;
                    depthBuf[idx] = depth;
                    if (depthOnly) continue;

                    Vec3 worldPos = world[0] * w[0] + world[1] * w[1] + world[2] * w[2];
                    Vec2 uvI = uv[0] * w[0] + uv[1] * w[1] + uv[2] * w[2];
                    Vec3 nrm = normalized(vnorm[0] * w[0] + vnorm[1] * w[1] + vnorm[2] * w[2]);
                    TGAColor tex = sampleTexture(sm.diffuse, uvI.x, uvI.y);
                    if (sm.normalMap && face.hasUV) {
                        TGAColor nm = sampleTexture(sm.normalMap, uvI.x, uvI.y);
                        Vec3 tangentN = {nm.r / 127.5 - 1.0, nm.g / 127.5 - 1.0, nm.b / 127.5 - 1.0};
                        nrm = normalized(T * tangentN.x + B * tangentN.y + N * tangentN.z);
                    }

                    bool inShadow = false;
                    if (haveShadow_) {
                        double lc[4];
                        transformPoint(lightViewProj_, worldPos, lc);
                        if (lc[3] > 0) {
                            double lx = (lc[0] / lc[3] + 1.0) * 0.5 * WIDTH;
                            double ly = (1.0 - lc[1] / lc[3]) * 0.5 * HEIGHT;
                            int sx = (int)lx, sy = (int)ly;
                            if (sx >= 0 && sx < WIDTH && sy >= 0 && sy < HEIGHT) {
                                double stored = shadowZbuf_[size_t(sy) * size_t(WIDTH) + size_t(sx)];
                                // Bias 0.05 (world/clip-w units) kills self-shadow acne;
                                // a real occluder differs by >> 0.05 (see TEST_PLAN shadow test).
                                if (lc[3] > stored + 0.05) inShadow = true;
                            }
                        }
                    }
                    outFb->set(x, y, phongShade(tex, nrm, worldPos, viewEye, light, inShadow));
                }
            }
        }
    }
}
