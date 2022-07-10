#pragma once

#include <array>
#include <string>

#include "ppm.hpp"

static const std::array<std::string, 11> sample_names {
    "camera2_hfyu32.avi",
    "camera2_png.avi",
    "rgb24_interlaced.avi",
    "rgb_predgrad.avi",
    "rgb_predleft.avi",
    "rgb_predleftnodecorr.avi",
    "rgb_v1.avi",
    "yuv_predgrad.avi",
    "yuv_predleft.avi",
    "yuv_predmed.avi",
    "yuv_v1.avi"
};

static const std::array<size_t, 11> sample_frames {
    30,
    30,
    2,
    42,
    42,
    42,
    42,
    42,
    42,
    42,
    42
};

std::unique_ptr<unsigned char[]> load_frame(int index_sample, int index_frame, int& width, int& height) {
    std::string frame = std::to_string(index_frame);
    frame = std::string(std::max(6 - (int)frame.size(), 0), '0') + frame;
    std::string path = "frames/" + sample_names[index_sample] + "/" + frame + ".ppm";
    return load_ppm(path, width, height);
}
