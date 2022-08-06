#pragma once

#include <array>
#include <string>

#include "ppm.hpp"

static const std::array<std::string, 10> sample_names {
    "camera2_hfyu32.avi",
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

static const std::array<size_t, 10> sample_frames {
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
    return ppm_load(path, width, height);
}

std::unique_ptr<unsigned char[]> load_video(int index_sample, int& length) {
    std::string path = "samples/" + sample_names[index_sample];
    std::FILE* handle = std::fopen(path.c_str(), "rb");
    if (!handle) {
        return nullptr;
    }
    fpos_t position;
    if (std::fgetpos(handle, &position) != 0) {
        std::fclose(handle);
        return nullptr;
    }
    if (std::fseek(handle, 0, SEEK_END) != 0) {
        std::fclose(handle);
        return nullptr;
    }
    long position_end = std::ftell(handle);
    if (position_end < 0) {
        std::fclose(handle);
        return nullptr;
    }
    if (std::fsetpos(handle, &position) != 0) {
        std::fclose(handle);
        return nullptr;
    }
    length = static_cast<size_t>(position_end);
    std::unique_ptr<unsigned char[]> buffer = std::unique_ptr<unsigned char[]>(new unsigned char[length]);
    size_t file_remaining = length;
    size_t file_read = file_remaining;
    while (std::fread(buffer.get() + (length - file_remaining), 1, file_read, handle) && (file_read != 0)) {
        file_remaining -= file_read;
        file_read = file_remaining;
    }
    std::fclose(handle);
    if (file_remaining != 0) {
        std::fprintf(stderr, "Failed to read file contents.\n");
        return nullptr;
    }
    return buffer;
}
