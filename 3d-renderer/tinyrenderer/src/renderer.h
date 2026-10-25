// renderer.h — framebuffer, depth buffer, triangle rasterization, shadow pass.
#pragma once
#include "geometry.h"
#include "model.h"
#include "shaders.h"
#include "tgaimage.h"
#include <vector>

struct SceneModel {
    const Model* model = nullptr;
    const TGAImage* diffuse = nullptr;   // nullptr = white
    const TGAImage* normalMap = nullptr; // nullptr = no normal mapping
};

class Renderer {
public:
    static const int WIDTH = 800;
    static const int HEIGHT = 800;
    TGAImage framebuffer;
    Renderer() : framebuffer(WIDTH, HEIGHT, TGAColor(0, 0, 0)) {}

    // Full render: optional light-view shadow pass, then camera pass.
    // eye/center/up define the camera; light defines the point light and
    // (when useShadow) the light viewpoint for the shadow depth buffer.
    void render(const std::vector<SceneModel>& models, const Vec3& eye,
                const Vec3& center, const Vec3& up, const Light& light, bool useShadow);
private:
    std::vector<double> zbuf_;        // camera depth buffer (view-space distance)
    std::vector<double> shadowZbuf_;  // light depth buffer
    Mat4 lightViewProj_;              // set when useShadow
    bool haveShadow_ = false;

    // Rasterize all models through one view/projection. If depthOnly, only
    // the given depth buffer is written (shadow pass); otherwise colours
    // are shaded into framebuffer_ and zbuf_ is updated.
    void rasterizePass(const std::vector<SceneModel>& models, const Mat4& viewProj,
                       const Vec3& viewEye, const Light& light, bool depthOnly,
                       std::vector<double>& depthBuf, TGAImage* outFb);
};
