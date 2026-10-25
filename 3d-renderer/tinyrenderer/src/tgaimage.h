// tgaimage.h — minimal TGA image class (own implementation, not copied
// from the tutorial). Supports uncompressed truecolor (type 2, 24/32-bit)
// and grayscale (type 3, 8-bit) on read; writes uncompressed truecolor
// type 2, 24-bit, top-left origin. RLE TGAs are rejected on read.
#pragma once
#include <cstdint>
#include <string>
#include <vector>

struct TGAColor {
    uint8_t r = 0, g = 0, b = 0, a = 255;
    TGAColor() = default;
    TGAColor(uint8_t r_, uint8_t g_, uint8_t b_, uint8_t a_ = 255) : r(r_), g(g_), b(b_), a(a_) {}
};

class TGAImage {
public:
    int width = 0, height = 0;
    TGAImage() = default;
    TGAImage(int w, int h, TGAColor fill = TGAColor());
    TGAColor get(int x, int y) const;              // (0,0) = top-left
    void set(int x, int y, TGAColor c);
    bool read(const std::string& path, std::string& err);
    bool write(const std::string& path, std::string& err) const;
private:
    std::vector<TGAColor> data_;  // row-major, top row first
};
