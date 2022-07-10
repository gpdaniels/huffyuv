#pragma once

#include <cstdio>
#include <memory>
#include <string>

std::unique_ptr<unsigned char[]> load_ppm(const std::string& filepath, int& width, int& height) {
    std::FILE* handle = std::fopen(filepath.c_str(), "rb");
    if (!handle) {
        return nullptr;
    }
    char P6[2]{};
    std::fscanf(handle, "%c%c", &P6[0], &P6[1]);
    if ((P6[0] != 'P') || (P6[1] != '6')) {
        fclose(handle);
        return nullptr;
    }
    std::fscanf(handle, "%d\n %d\n", &width, &height);
    int maximum;
    std::fscanf(handle, "%d\n", &maximum);
    const size_t size = width * height;
    std::unique_ptr<unsigned char[]> pixels = std::unique_ptr<unsigned char[]>(new unsigned char[size]);
    size_t index = 0;
    while (index < size) {
        const size_t delta = fread(&pixels[index], sizeof(char), size - index, handle);
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
