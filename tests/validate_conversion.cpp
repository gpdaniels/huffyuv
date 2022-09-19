#include <huffyuv.hpp>

#include "convert.hpp"
#include "samples.hpp"

int main(int argc, char* argv[]) {
    static_cast<void>(argc);
    static_cast<void>(argv);

    // Validate extracted frames.
    for (size_t index_sample = 0; index_sample < sample_names.size(); ++index_sample) {
        for (size_t index_frame = 0; index_frame < sample_frames[index_sample]; ++index_frame) {
            size_t width = 0;
            size_t height = 0;
            std::unique_ptr<unsigned char[]> pixels_loaded = load_frame(index_sample, index_frame, width, height);
            if ((pixels_loaded == nullptr) || (width == 0) || (height == 0)) {
                fprintf(stderr, "Failed read frame %zu for sample '%s'.\n", sample_frames[index_sample], sample_names[index_sample].c_str());
                return 1;
            }
            {
                std::unique_ptr<unsigned char[]> pixels_rgba = rgb_to_rgba(width, height, pixels_loaded.get());
                std::unique_ptr<unsigned char[]> pixels_rgb = rgba_to_rgb(width, height, pixels_rgba.get());
                constexpr static const size_t channels = 3;
                for (size_t y = 0; y < height; ++y) {
                    for (size_t x = 0; x < width; ++x) {
                        for (size_t channel = 0; channel < channels; ++channel) {
                            const size_t index_pixel = y * width * channels + x * channels + channel;
                            const unsigned char loaded_value = pixels_loaded.get()[index_pixel];
                            const unsigned char decoded_value = pixels_rgb.get()[index_pixel];
                            if (loaded_value != decoded_value) {
                                fprintf(stderr, "Failed to convert '%s' frame %zu rgb to rgba.\n", sample_names[index_sample].c_str(), index_frame);
                                return 1;
                            }
                        }
                    }
                }
            }
            {
                std::unique_ptr<unsigned char[]> pixels = rgb_to_rgba(width, height, pixels_loaded.get());
                std::unique_ptr<unsigned char[]> pixels_bgra = rgba_to_bgra(width, height, pixels.get());
                std::unique_ptr<unsigned char[]> pixels_rgba = bgra_to_rgba(width, height, pixels_bgra.get());
                constexpr static const size_t channels = 3;
                for (size_t y = 0; y < height; ++y) {
                    for (size_t x = 0; x < width; ++x) {
                        for (size_t channel = 0; channel < channels; ++channel) {
                            const size_t index_pixel = y * width * channels + x * channels + channel;
                            const unsigned char loaded_value = pixels.get()[index_pixel];
                            const unsigned char decoded_value = pixels_rgba.get()[index_pixel];
                            if (loaded_value != decoded_value) {
                                fprintf(stderr, "Failed to convert '%s' frame %zu rgba to bgra.\n", sample_names[index_sample].c_str(), index_frame);
                                return 1;
                            }
                        }
                    }
                }
            }
            {
                std::unique_ptr<unsigned char[]> pixels_bgr = rgb_to_bgr(width, height, pixels_loaded.get());
                std::unique_ptr<unsigned char[]> pixels_rgb = bgr_to_rgb(width, height, pixels_bgr.get());
                constexpr static const size_t channels = 3;
                for (size_t y = 0; y < height; ++y) {
                    for (size_t x = 0; x < width; ++x) {
                        for (size_t channel = 0; channel < channels; ++channel) {
                            const size_t index_pixel = y * width * channels + x * channels + channel;
                            const unsigned char loaded_value = pixels_loaded.get()[index_pixel];
                            const unsigned char decoded_value = pixels_rgb.get()[index_pixel];
                            if (loaded_value != decoded_value) {
                                fprintf(stderr, "Failed to convert '%s' frame %zu rgb to bgr.\n", sample_names[index_sample].c_str(), index_frame);
                                return 1;
                            }
                        }
                    }
                }
            }
            {
                std::unique_ptr<unsigned char[]> pixels_yuv = rgb_to_yuv(width, height, pixels_loaded.get());
                std::unique_ptr<unsigned char[]> pixels_rgb = yuv_to_rgb(width, height, pixels_yuv.get());
                constexpr static const size_t channels = 3;
                for (size_t y = 0; y < height; ++y) {
                    for (size_t x = 0; x < width; ++x) {
                        for (size_t channel = 0; channel < channels; ++channel) {
                            const size_t index_pixel = y * width * channels + x * channels + channel;
                            const unsigned char loaded_value = pixels_loaded.get()[index_pixel];
                            const unsigned char decoded_value = pixels_rgb.get()[index_pixel];
                            if (std::abs(loaded_value - decoded_value) > sample_yuv_rgb_error[index_sample][channel]) {
                                fprintf(stderr, "Failed to convert '%s' frame %zu rgb to yuv.\n", sample_names[index_sample].c_str(), index_frame);
                                return 1;
                            }
                        }
                    }
                }
            }
        }
    }
    return 0;
}
