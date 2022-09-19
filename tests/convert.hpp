#pragma once

#include <memory>

inline std::unique_ptr<unsigned char[]> rgb_to_rgba(size_t width, size_t height, const unsigned char* pixels_rgb) {
    std::unique_ptr<unsigned char[]> pixels_rgba = std::unique_ptr<unsigned char[]>(new unsigned char[width * height * 4]);
    for (size_t y = 0; y < height; ++y) {
        for (size_t x = 0; x < width; ++x) {
            for (size_t c = 0; c < 3; ++c) {
                pixels_rgba[y * width * 4 + x * 4 + c] = pixels_rgb[y * width * 3 + x * 3 + c];
            }
            // Default alpha to max.
            pixels_rgba[y * width * 4 + x * 4 + 3] = 255;
        }
    }
    return pixels_rgba;
}

inline std::unique_ptr<unsigned char[]> rgba_to_rgb(size_t width, size_t height, const unsigned char* pixels_rgba) {
    std::unique_ptr<unsigned char[]> pixels_rgb = std::unique_ptr<unsigned char[]>(new unsigned char[width * height * 3]);
    for (size_t y = 0; y < height; ++y) {
        for (size_t x = 0; x < width; ++x) {
            for (size_t c = 0; c < 3; ++c) {
                pixels_rgb[y * width * 3 + x * 3 + c] = pixels_rgba[y * width * 4 + x * 4 + c];
            }
        }
    }
    return pixels_rgb;
}

inline std::unique_ptr<unsigned char[]> bgra_to_rgba(size_t width, size_t height, const unsigned char* pixels_bgra) {
    std::unique_ptr<unsigned char[]> pixels_rgba = std::unique_ptr<unsigned char[]>(new unsigned char[width * height * 4]);
    for (size_t y = 0; y < height; ++y) {
        for (size_t x = 0; x < width; ++x) {
            pixels_rgba[y * width * 4 + x * 4 + 0] = pixels_bgra[y * width * 4 + x * 4 + 2];
            pixels_rgba[y * width * 4 + x * 4 + 1] = pixels_bgra[y * width * 4 + x * 4 + 1];
            pixels_rgba[y * width * 4 + x * 4 + 2] = pixels_bgra[y * width * 4 + x * 4 + 0];
            pixels_rgba[y * width * 4 + x * 4 + 3] = pixels_bgra[y * width * 4 + x * 4 + 3];
        }
    }
    return pixels_rgba;
}

inline std::unique_ptr<unsigned char[]> rgba_to_bgra(size_t width, size_t height, const unsigned char* pixels_rgba) {
    std::unique_ptr<unsigned char[]> pixels_bgra = std::unique_ptr<unsigned char[]>(new unsigned char[width * height * 4]);
    for (size_t y = 0; y < height; ++y) {
        for (size_t x = 0; x < width; ++x) {
            pixels_bgra[y * width * 4 + x * 4 + 0] = pixels_rgba[y * width * 4 + x * 4 + 2];
            pixels_bgra[y * width * 4 + x * 4 + 1] = pixels_rgba[y * width * 4 + x * 4 + 1];
            pixels_bgra[y * width * 4 + x * 4 + 2] = pixels_rgba[y * width * 4 + x * 4 + 0];
            pixels_bgra[y * width * 4 + x * 4 + 3] = pixels_rgba[y * width * 4 + x * 4 + 3];
        }
    }
    return pixels_bgra;
}

inline std::unique_ptr<unsigned char[]> bgr_to_rgb(size_t width, size_t height, const unsigned char* pixels_bgr) {
    std::unique_ptr<unsigned char[]> pixels_rgb = std::unique_ptr<unsigned char[]>(new unsigned char[width * height * 3]);
    for (size_t y = 0; y < height; ++y) {
        for (size_t x = 0; x < width; ++x) {
            pixels_rgb[y * width * 3 + x * 3 + 0] = pixels_bgr[y * width * 3 + x * 3 + 2];
            pixels_rgb[y * width * 3 + x * 3 + 1] = pixels_bgr[y * width * 3 + x * 3 + 1];
            pixels_rgb[y * width * 3 + x * 3 + 2] = pixels_bgr[y * width * 3 + x * 3 + 0];
        }
    }
    return pixels_rgb;
}

inline std::unique_ptr<unsigned char[]> rgb_to_bgr(size_t width, size_t height, const unsigned char* pixels_rgb) {
    std::unique_ptr<unsigned char[]> pixels_bgr = std::unique_ptr<unsigned char[]>(new unsigned char[width * height * 3]);
    for (size_t y = 0; y < height; ++y) {
        for (size_t x = 0; x < width; ++x) {
            pixels_bgr[y * width * 3 + x * 3 + 0] = pixels_rgb[y * width * 3 + x * 3 + 2];
            pixels_bgr[y * width * 3 + x * 3 + 1] = pixels_rgb[y * width * 3 + x * 3 + 1];
            pixels_bgr[y * width * 3 + x * 3 + 2] = pixels_rgb[y * width * 3 + x * 3 + 0];
        }
    }
    return pixels_bgr;
}

inline std::unique_ptr<unsigned char[]> rgb_to_yuv(size_t width, size_t height, const unsigned char* pixels_rgb) {
    constexpr static const auto clip = [](int value) {
        const int shifted = (320 + ((value + 0x8000) >> 16));
        if (shifted < 320) {
            return 0;
        }
        else if (shifted < 320 + 256) {
            return (shifted - 320);
        }
        return 255;
    };

    constexpr static const int cyb = static_cast<int>(0.114 * 219 / 255 * 65536 + 0.5);
    constexpr static const int cyg = static_cast<int>(0.587 * 219 / 255 * 65536 + 0.5);
    constexpr static const int cyr = static_cast<int>(0.299 * 219 / 255 * 65536 + 0.5);

    std::unique_ptr<unsigned char[]> pixels_yuv = std::unique_ptr<unsigned char[]>(new unsigned char[width * height * 4]);
    for (size_t y = 0; y < height; ++y) {
        for (size_t x = 0; x < width; x += 2) {
            const unsigned char r0 = pixels_rgb[y * width * 3 + x * 3 + 0];
            const unsigned char g0 = pixels_rgb[y * width * 3 + x * 3 + 1];
            const unsigned char b0 = pixels_rgb[y * width * 3 + x * 3 + 2];
            const unsigned char r1 = pixels_rgb[y * width * 3 + x * 3 + 3];
            const unsigned char g1 = pixels_rgb[y * width * 3 + x * 3 + 4];
            const unsigned char b1 = pixels_rgb[y * width * 3 + x * 3 + 5];

            const int y0 = (cyr * r0 + cyg * g0 + cyb * b0 + 0x108000) >> 16;
            const int y1 = (cyr * r1 + cyg * g1 + cyb * b1 + 0x108000) >> 16;
            const int scaled_y = (y0 + y1 - 32) * static_cast<int>(255.0 / 219.0 * 32768 + 0.5);
            const int ry = ((r0 + r1) << 15) - scaled_y;
            const int by = ((b0 + b1) << 15) - scaled_y;

            pixels_yuv[y * width * 2 + x * 2 + 0] = static_cast<unsigned char>(y0);
            pixels_yuv[y * width * 2 + x * 2 + 1] = static_cast<unsigned char>(clip((by >> 10) * static_cast<int>(1 / 2.018 * 1024 + 0.5) + 0x800000));
            pixels_yuv[y * width * 2 + x * 2 + 2] = static_cast<unsigned char>(y1);
            pixels_yuv[y * width * 2 + x * 2 + 3] = static_cast<unsigned char>(clip((ry >> 10) * static_cast<int>(1 / 1.596 * 1024 + 0.5) + 0x800000));
        }
    }

    return pixels_yuv;
}

inline std::unique_ptr<unsigned char[]> yuv_to_rgb(size_t width, size_t height, const unsigned char* pixels_yuv) {
    constexpr static const auto clip = [](int value) {
        const int shifted = (320 + ((value + 0x8000) >> 16));
        if (shifted < 320) {
            return 0;
        }
        else if (shifted < 320 + 256) {
            return (shifted - 320);
        }
        return 255;
    };

    constexpr static const int crv = static_cast<int>(1.596 * 65536 + 0.5);
    constexpr static const int cgv = static_cast<int>(0.813 * 65536 + 0.5);
    constexpr static const int cgu = static_cast<int>(0.391 * 65536 + 0.5);
    constexpr static const int cbu = static_cast<int>(2.018 * 65536 + 0.5);

    std::unique_ptr<unsigned char[]> pixels_rgb = std::unique_ptr<unsigned char[]>(new unsigned char[width * height * 3]);
    for (size_t y = 0; y < height; ++y) {
        for (size_t x = 0; x < width; x += 2) {
            const int y0 = static_cast<int>(pixels_yuv[y * width * 2 + x * 2 + 0]);
            const int u0 = static_cast<int>(pixels_yuv[y * width * 2 + x * 2 + 1]);
            const int y1 = static_cast<int>(pixels_yuv[y * width * 2 + x * 2 + 2]);
            const int v0 = static_cast<int>(pixels_yuv[y * width * 2 + x * 2 + 3]);

            const int scaled_y0 = (y0 - 16) * static_cast<int>((255.0 / 219.0) * 65536 + 0.5);
            const int scaled_y1 = (y1 - 16) * static_cast<int>((255.0 / 219.0) * 65536 + 0.5);

            pixels_rgb[y * width * 3 + x * 3 + 0] = static_cast<unsigned char>(clip(scaled_y0 + (v0 - 128) * crv));
            pixels_rgb[y * width * 3 + x * 3 + 1] = static_cast<unsigned char>(clip(scaled_y0 - (v0 - 128) * cgv - (u0 - 128) * cgu));
            pixels_rgb[y * width * 3 + x * 3 + 2] = static_cast<unsigned char>(clip(scaled_y0 + (u0 - 128) * cbu));
            pixels_rgb[y * width * 3 + x * 3 + 3] = static_cast<unsigned char>(clip(scaled_y1 + (v0 - 128) * crv));
            pixels_rgb[y * width * 3 + x * 3 + 4] = static_cast<unsigned char>(clip(scaled_y1 - (v0 - 128) * cgv - (u0 - 128) * cgu));
            pixels_rgb[y * width * 3 + x * 3 + 5] = static_cast<unsigned char>(clip(scaled_y1 + (u0 - 128) * cbu));
        }
    }
    return pixels_rgb;
}
