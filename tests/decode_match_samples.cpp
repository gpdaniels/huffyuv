#include <huffyuv.hpp>

#include "samples.hpp"
#include "convert.hpp"

int main(int argc, char* argv[]) {
    static_cast<void>(argc);
    static_cast<void>(argv);

    for (size_t index_sample = 0; index_sample < sample_names.size(); ++index_sample) {
        size_t length = 0;
        std::unique_ptr<unsigned char[]> file = load_video(index_sample, length);
        huffyuv video;
        if ((file != nullptr) && (length != 0) && (!video.parse(file.get(), length))) {
            fprintf(stderr, "Failed to parse avi of sample '%s'.\n", sample_names[index_sample].c_str());
            return 1;
        }

        // Check the number of frames is the same.
        if (video.get_frames().size() != sample_frames[index_sample]) {
            fprintf(stderr, "Failed to parse avi of sample '%s', number of frames does not match.\n", sample_names[index_sample].c_str());
            return 1;
        }

        // Check that all the ffmpeg decoded frames match.
        for (size_t index_frame = 0; index_frame < sample_frames[index_sample]; ++index_frame) {
            size_t width = 0;
            size_t height = 0;
            std::unique_ptr<unsigned char[]> pixels_loaded = load_frame(index_sample, index_frame, width, height);
            if ((pixels_loaded == nullptr) || (width == 0) || (height == 0)) {
                fprintf(stderr, "Failed read frame %zu/%zu for sample '%s'.\n", index_frame, sample_frames[index_sample], sample_names[index_sample].c_str());
                return 1;
            }
            // TODO: Problem, can't compare RGBA alpha when using ppm as it doesn't store alpha
            pixels_loaded = rgb_to_rgba(width, height, pixels_loaded.get());

            avi::frame_type frame = video.get_frames_uncompressed()[index_frame];
            if ((width != static_cast<size_t>(video.get_frame_header().width)) || (height != static_cast<size_t>(video.get_frame_header().height))) {
                fprintf(stderr, "Failed read frame %zu/%zu for sample '%s', frame dimensions do not match.\n", index_frame, sample_frames[index_sample], sample_names[index_sample].c_str());
                return 1;
            }
            std::unique_ptr<unsigned char[]> pixels_decoded;
            if (video.get_bit_count() == 32) {
                if (frame.data.size() != width * height * 4) {
                    fprintf(stderr, "Invalid frame data size frame %zu/%zu for sample '%s'.\n", index_frame, sample_frames[index_sample], sample_names[index_sample].c_str());
                    return 1;
                }
                pixels_decoded = bgra_to_rgba(width, height, frame.data.data());
            }
            else if (video.get_bit_count() == 24) {
                if (frame.data.size() != width * height * 3) {
                    fprintf(stderr, "Invalid frame data size frame %zu/%zu for sample '%s'.\n", index_frame, sample_frames[index_sample], sample_names[index_sample].c_str());
                    return 1;
                }
                pixels_decoded = bgr_to_rgb(width, height, frame.data.data());
                pixels_decoded = rgb_to_rgba(width, height, pixels_decoded.get());
            }
            else if (video.get_bit_count() == 16) {
                if (frame.data.size() != width * height * 2) {
                    fprintf(stderr, "Invalid frame data size frame %zu/%zu for sample '%s'.\n", index_frame, sample_frames[index_sample], sample_names[index_sample].c_str());
                    return 1;
                }
                pixels_decoded = yuv_to_rgb(width, height, frame.data.data());
                pixels_decoded = rgb_to_rgba(width, height, pixels_decoded.get());
            }
            else {
                fprintf(stderr, "Unknown bit count in frame %zu/%zu for sample '%s'.\n", index_frame, sample_frames[index_sample], sample_names[index_sample].c_str());
                return 1;
            }

            // Everything should be in rgba mode by here.
            constexpr static const size_t channels = 4;

            bool alpha_warning = false;

            // Compare all pixels.
            for (size_t y = 0; y < height; ++y) {
                for (size_t x = 0; x < width; ++x) {
                    for (size_t channel = 0; channel < channels; ++channel) {
                        const size_t index_pixel = y * width * channels + x * channels + channel;
                        const unsigned char loaded_value = pixels_loaded.get()[index_pixel];
                        const unsigned char decoded_value = pixels_decoded.get()[index_pixel];
                        if (loaded_value != decoded_value) {
                            // Only warn for alpha differences.
                            if (channel == 3) {
                                if (!alpha_warning) {
                                    fprintf(stderr, "Warning, alpha does not match on frame %zu for sample '%s'.\n", index_frame, sample_names[index_sample].c_str());
                                    alpha_warning = true;
                                }
                                continue;
                            }
                            // Allow a tolerance for yuv differences.
                            if (video.get_bit_count() == 16) {
                                if (std::abs(loaded_value - decoded_value) <= sample_yuv_rgb_error[index_sample][channel]) {
                                    continue;
                                }
                            }
                            fprintf(stderr, "Failed to match frame %zu for sample '%s'.\n", index_frame, sample_names[index_sample].c_str());
                            return 1;
                        }
                    }
                }
            }
        }
    }
    
    return 0;
}
