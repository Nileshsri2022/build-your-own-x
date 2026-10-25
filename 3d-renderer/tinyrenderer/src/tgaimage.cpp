#include "tgaimage.h"
#include <cstdio>

TGAImage::TGAImage(int w, int h, TGAColor fill) : width(w), height(h), data_(size_t(w) * size_t(h), fill) {}

TGAColor TGAImage::get(int x, int y) const {
    if (x < 0 || y < 0 || x >= width || y >= height) return TGAColor();
    return data_[size_t(y) * size_t(width) + size_t(x)];
}

void TGAImage::set(int x, int y, TGAColor c) {
    if (x < 0 || y < 0 || x >= width || y >= height) return;
    data_[size_t(y) * size_t(width) + size_t(x)] = c;
}

bool TGAImage::read(const std::string& path, std::string& err) {
    FILE* f = std::fopen(path.c_str(), "rb");
    if (!f) { err = "cannot open file: " + path; return false; }
    uint8_t hdr[18];
    if (std::fread(hdr, 1, 18, f) != 18) { std::fclose(f); err = "short TGA header: " + path; return false; }
    int idLen = hdr[0], cmapType = hdr[1], imgType = hdr[2];
    int w = hdr[12] | (hdr[13] << 8), h = hdr[14] | (hdr[15] << 8), bpp = hdr[16];
    uint8_t desc = hdr[17];
    if (imgType != 2 && imgType != 3) { std::fclose(f); err = "unsupported TGA type (only uncompressed 2/3): " + path; return false; }
    if (imgType == 2 && bpp != 24 && bpp != 32) { std::fclose(f); err = "unsupported truecolor bpp (24/32 only): " + path; return false; }
    if (imgType == 3 && bpp != 8) { std::fclose(f); err = "unsupported grayscale bpp (8 only): " + path; return false; }
    if (cmapType != 0) { std::fclose(f); err = "colormapped TGA not supported: " + path; return false; }
    if (idLen) std::fseek(f, idLen, SEEK_CUR);
    int bytesPer = bpp / 8;
    std::vector<uint8_t> raw(size_t(w) * size_t(h) * size_t(bytesPer));
    if (std::fread(raw.data(), 1, raw.size(), f) != raw.size()) { std::fclose(f); err = "short TGA pixel data: " + path; return false; }
    std::fclose(f);
    width = w; height = h;
    data_.assign(size_t(w) * size_t(h), TGAColor());
    bool topOrigin = (desc & 0x20) != 0;
    for (int y = 0; y < h; y++) {
        int fileRow = topOrigin ? y : (h - 1 - y);
        for (int x = 0; x < w; x++) {
            size_t i = (size_t(fileRow) * size_t(w) + size_t(x)) * size_t(bytesPer);
            TGAColor c;
            if (imgType == 3) { c = TGAColor(raw[i], raw[i], raw[i]); }
            else { c = TGAColor(raw[i + 2], raw[i + 1], raw[i], bytesPer == 4 ? raw[i + 3] : 255); }
            data_[size_t(y) * size_t(w) + size_t(x)] = c;
        }
    }
    return true;
}

bool TGAImage::write(const std::string& path, std::string& err) const {
    FILE* f = std::fopen(path.c_str(), "wb");
    if (!f) { err = "cannot open output file: " + path; return false; }
    uint8_t hdr[18] = {};
    hdr[2] = 2;  // uncompressed truecolor
    hdr[12] = width & 0xff; hdr[13] = (width >> 8) & 0xff;
    hdr[14] = height & 0xff; hdr[15] = (height >> 8) & 0xff;
    hdr[16] = 24;
    hdr[17] = 0x20;  // top-left origin
    bool ok = std::fwrite(hdr, 1, 18, f) == 18;
    for (int y = 0; ok && y < height; y++)
        for (int x = 0; x < width; x++) {
            TGAColor c = get(x, y);
            uint8_t px[3] = {c.b, c.g, c.r};
            if (std::fwrite(px, 1, 3, f) != 3) { ok = false; break; }
        }
    if (std::fclose(f) != 0) ok = false;
    if (!ok) err = "failed writing TGA: " + path;
    return ok;
}
