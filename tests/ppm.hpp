#pragma once

#include <cstdio>
#include <memory>
#include <string>

inline bool ppm_save(const std::string& filepath, size_t width, size_t height, const unsigned char* pixels) {
    std::FILE* handle = std::fopen(filepath.c_str(), "wb");
    if (!handle) {
        return false;
    }
    if (std::fprintf(handle, "P6\n") != 3) {
        fclose(handle);
        return false;
    }
    if (std::fprintf(handle, "%zu %zu\n", width, height) <= 0) {
        fclose(handle);
        return false;
    }
    if (std::fprintf(handle, "%d\n", 255) <= 0) {
        fclose(handle);
        return false;
    }
    const size_t size = width * height * 3;
    size_t index = 0;
    while (index < size) {
        const size_t delta = std::fwrite(&pixels[index], sizeof(char), size - index, handle);
        if (delta == 0) {
            break;
        }
        index += delta;
    }
    if (index != size) {
        return false;
    }
    std::fclose(handle);
    return true;
}

inline std::unique_ptr<unsigned char[]> ppm_load(const std::string& filepath, size_t& width, size_t& height) {
    std::FILE* handle = std::fopen(filepath.c_str(), "rb");
    if (!handle) {
        return nullptr;
    }
    char P6[3]{};
    if (std::fscanf(handle, "%c%c%c", &P6[0], &P6[1], &P6[2]) != 3) {
        fclose(handle);
        return nullptr;
    }
    if ((P6[0] != 'P') || (P6[1] != '6') || (P6[2] != '\n')) {
        fclose(handle);
        return nullptr;
    }
    if (std::fscanf(handle, "%zu %zu\n", &width, &height) != 2) {
        fclose(handle);
        return nullptr;
    }
    int maximum;
    if (std::fscanf(handle, "%d\n", &maximum) != 1) {
        fclose(handle);
        return nullptr;
    }
    const size_t size = width * height * 3;
    std::unique_ptr<unsigned char[]> pixels = std::unique_ptr<unsigned char[]>(new unsigned char[size]);
    size_t index = 0;
    while (index < size) {
        const size_t delta = std::fread(&pixels[index], sizeof(char), size - index, handle);
        if (delta == 0) {
            break;
        }
        index += delta;
    }
    if (index != size) {
        return nullptr;
    }
    std::fclose(handle);
    return pixels;
}
