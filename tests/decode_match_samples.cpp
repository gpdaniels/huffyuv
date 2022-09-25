#include <avi.hpp>
#include <huffyuv.hpp>

#include "samples.hpp"
#include "convert.hpp"

int main(int argc, char* argv[]) {
    static_cast<void>(argc);
    static_cast<void>(argv);

    for (size_t index_sample = 0; index_sample < sample_names.size(); ++index_sample) {
        // Load avi.
        size_t length = 0;
        std::unique_ptr<unsigned char[]> file = load_video(index_sample, length);
        if ((file == nullptr) || (length == 0)) {
            fprintf(stderr, "Failed to load avi of sample '%s'.\n", sample_names[index_sample].c_str());
            return 1;
        }

        // Decode avi.
        avi video;
        if (!video.parse(file.get(), length)) {
            std::fprintf(stderr, "Failed to parse avi of sample '%s'.\n", sample_names[index_sample].c_str());
            return 1;
        }

        // Select stream.
        unsigned int stream_number = 0xFFFFFFFF;
        for (unsigned int i = 0; i < video.get_strhs().size(); ++i) {
            const avi::strh_type* strh = video.get_strhs()[i];
            if (
                (strh->type == avi::fourcc("vids")) &&
                ((strh->handler == avi::fourcc("hfyu")) || (strh->handler == avi::fourcc("HFYU")))
            ) {
                const avi::strf_type& strf = video.get_strfs()[i];
                if (
                    (strf.identifier == avi::fourcc("vids")) &&
                    (strf.strf_vids->compression_identifier == avi::fourcc("HFYU"))
                ) {
                    stream_number = i;
                    break;
                }
            }
        }
        if (stream_number == 0xFFFFFFFF) {
            std::fprintf(stderr, "Failed find a HFYU encoded video stream inside the avi file.\n");
            return 1;
        }

        // Check the number of frames is the same.
        if (video.get_frames()[stream_number].size() != sample_frames[index_sample]) {
            fprintf(stderr, "Failed to parse avi of sample '%s', number of frames does not match.\n", sample_names[index_sample].c_str());
            return 1;
        }

        // Setup codec.
        huffyuv codec(reinterpret_cast<const unsigned char*>(video.get_strfs()[stream_number].strf_vids), video.get_strfs()[stream_number].strf_vids->header_size);
        if (!codec.is_valid()) {
            fprintf(stderr, "Failed setup huffyuv for for sample '%s'.\n", sample_names[index_sample].c_str());
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

            if ((width != static_cast<size_t>(video.get_strfs()[stream_number].strf_vids->width)) || (height != static_cast<size_t>(video.get_strfs()[stream_number].strf_vids->height))) {
                fprintf(stderr, "Failed read frame %zu/%zu for sample '%s', frame dimensions do not match.\n", index_frame, sample_frames[index_sample], sample_names[index_sample].c_str());
                return 1;
            }

            unsigned long long int pixels_decoded_length = codec.get_decoded_image_size();
            std::unique_ptr<unsigned char[]> pixels_decoded = std::unique_ptr<unsigned char[]>(new unsigned char[pixels_decoded_length]);
            codec.decode(
                video.get_frames()[stream_number][index_frame].data,
                video.get_frames()[stream_number][index_frame].length,
                pixels_decoded.get(),
                pixels_decoded_length
            );

            switch (codec.get_image_format()) {
                case huffyuv::format_type::yuyv: {
                    if (pixels_decoded_length != width * height * 2) {
                        fprintf(stderr, "Invalid frame data size frame %zu/%zu for sample '%s'.\n", index_frame, sample_frames[index_sample], sample_names[index_sample].c_str());
                        return 1;
                    }
                    pixels_decoded = yuv_to_rgb(width, height, pixels_decoded.get());
                    pixels_decoded = rgb_to_rgba(width, height, pixels_decoded.get());
                } break;
                case huffyuv::format_type::bgr: {
                    if (pixels_decoded_length != width * height * 3) {
                        fprintf(stderr, "Invalid frame data size frame %zu/%zu for sample '%s'.\n", index_frame, sample_frames[index_sample], sample_names[index_sample].c_str());
                        return 1;
                    }
                    pixels_decoded = bgr_to_rgb(width, height, pixels_decoded.get());
                    pixels_decoded = rgb_to_rgba(width, height, pixels_decoded.get());
                } break;
                case huffyuv::format_type::bgra: {
                    if (pixels_decoded_length != width * height * 4) {
                        fprintf(stderr, "Invalid frame data size frame %zu/%zu for sample '%s'.\n", index_frame, sample_frames[index_sample], sample_names[index_sample].c_str());
                        return 1;
                    }
                    pixels_decoded = bgra_to_rgba(width, height, pixels_decoded.get());
                } break;
                default: {
                    fprintf(stderr, "Unknown bit count in frame %zu/%zu for sample '%s'.\n", index_frame, sample_frames[index_sample], sample_names[index_sample].c_str());
                    return 1;
                }
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
                            if (codec.get_image_format() == huffyuv::format_type::yuyv) {
                                if (std::abs(loaded_value - decoded_value) <= sample_yuv_rgb_error[index_sample][channel]) {
                                    continue;
                                }
                            }
                            fprintf(stderr, "Failed to match frame %zu for sample '%s'.\n", index_frame, sample_names[index_sample].c_str());
                            ppm_save("pixels_loaded.ppm", width, height, rgba_to_rgb(width, height, pixels_loaded.get()).get());
                            ppm_save("pixels_decoded.ppm", width, height, rgba_to_rgb(width, height, pixels_decoded.get()).get());
                            return 1;
                        }
                    }
                }
            }
        }
    }
    
    return 0;
}
