// Ch. 5 — Film: per-pixel sample accumulation and PPM output.
// OUTPUT DECISION (SPEC §5): PPM (P3 ASCII) — our proposal, NOT a
// format claimed from the book. No tone mapping / gamma is applied:
// values are clamped to [0,1] and scaled to 0..255 (DESIGN: no tone
// mapping unless Ch. 5/Ch. 13 require it; chapter bodies not read).
#pragma once
#include <vector>
#include <fstream>
#include <string>
#include "radiometry.h"

namespace pbr {

class Film {
public:
    Film(int w, int h) : w_(w), h_(h), pixels_(w * h), counts_(w * h, 0) {}
    void addSample(int x, int y, const Spectrum& s) {
        int i = y * w_ + x;
        pixels_[i] += s; counts_[i] += 1;
    }
    Spectrum pixel(int x, int y) const {
        int i = y * w_ + x;
        return counts_[i] ? pixels_[i] / (double)counts_[i] : Spectrum{};
    }
    bool writePPM(const std::string& path) const {
        std::ofstream out(path);
        if (!out) return false;
        out << "P3\n" << w_ << ' ' << h_ << "\n255\n";
        for (int y = 0; y < h_; ++y) {
            for (int x = 0; x < w_; ++x) {
                Spectrum c = clamp01(pixel(x, y));
                out << (int)(c.x * 255) << ' ' << (int)(c.y * 255) << ' ' << (int)(c.z * 255) << '\n';
            }
        }
        return (bool)out;
    }
    int width() const { return w_; }
    int height() const { return h_; }
private:
    int w_, h_;
    std::vector<Spectrum> pixels_;
    std::vector<int> counts_;
};

}  // namespace pbr
