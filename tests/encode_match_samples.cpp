#include <huffyuv.hpp>

#include "avi.hpp"
#include "convert.hpp"
#include "samples.hpp"

int main(int argc, char* argv[]) {
    static_cast<void>(argc);
    static_cast<void>(argv);
    
    for (size_t index_sample = 0; index_sample < sample_names.size(); ++index_sample) {

        // Get frame details.
        size_t video_width = 0;
        size_t video_height = 0;
        load_frame(index_sample, 0, video_width, video_height);

        // Setup encode codec.
        huffyuv codec_encode(
            video_width,
            video_height,
            video_height > huffyuv::interlaced_threshold,
            true,
            huffyuv::format_type::bgr,
            huffyuv::predictor_type::left
        );
        if (!codec_encode.is_valid()) {
            fprintf(stderr, "Failed setup huffyuv encoder for sample '%s'.\n", sample_names[index_sample].c_str());
            return 1;
        }

        // Create/Prepare an avi file.
        constexpr static const unsigned int frame_rate = 25;

        avi video_encode;

        avi::avih_type avih;
        avih.microseconds_per_frame = 1000000 / frame_rate;
        avih.max_bytes_per_seccond = 0;
        avih.padding_granularity = 1;
        avih.flags = 0;
        avih.total_frames = sample_frames[index_sample];
        avih.initial_frames = 0;
        avih.stream_count = 1;
        avih.suggested_buffer_size = 0;
        avih.width = video_width;
        avih.height = video_height;

        avi::strh_type strh;
        strh.type = avi::fourcc("vids");
        strh.handler = avi::fourcc("hfyu");
        strh.flags = 0;
        strh.priority = 0;
        strh.language = 0;
        strh.initial_frames = 0;
        strh.scale = 1;
        strh.rate = frame_rate;
        strh.start = 0;
        strh.length = sample_frames[index_sample];
        strh.suggested_buffer_size = 0;
        strh.quality = 10000;
        strh.sample_size = 0;
        strh.destination.left = 0;
        strh.destination.top = 0;
        strh.destination.right = video_width;
        strh.destination.bottom = video_height;

        const unsigned int strf_vids_size = sizeof(avi::strf_vids_type) + 4 + codec_encode.get_packed_table_size();
        std::unique_ptr<unsigned char[]> strf_vids_data = std::unique_ptr<unsigned char[]>(new unsigned char[strf_vids_size]);
        if (!codec_encode.generate_stream_header(strf_vids_data.get(), strf_vids_size)) {
            fprintf(stderr, "Failed to generate avi stream header for sample '%s'.\n", sample_names[index_sample].c_str());
            return 1;
        }

        std::vector<std::unique_ptr<unsigned char[]>> frames_data;

        std::vector<avi::stream_type> streams;

        streams.push_back({});
        streams.back().strh = &strh;
        streams.back().strf_vids = reinterpret_cast<avi::strf_vids_type*>(strf_vids_data.get());

        // Add all the ffmpeg decoded frames.
        for (size_t index_frame = 0; index_frame < sample_frames[index_sample]; ++index_frame) {
            // Load frame data.
            size_t width = 0;
            size_t height = 0;
            std::unique_ptr<unsigned char[]> pixels = load_frame(index_sample, index_frame, width, height);
            if ((pixels == nullptr) || (width == 0) || (height == 0)) {
                fprintf(stderr, "Failed read frame %zu/%zu for sample '%s'.\n", index_frame, sample_frames[index_sample], sample_names[index_sample].c_str());
                return 1;
            }

            pixels = rgb_to_bgr(width, height, pixels.get());
            const unsigned long long int pixels_length = 3 * width * height;

            // Frame encoding using the huffyuv class.
            unsigned long long int pixels_encoded_length = codec_encode.get_decoded_image_size();
            std::unique_ptr<unsigned char[]> pixels_encoded = std::unique_ptr<unsigned char[]>(new unsigned char[pixels_encoded_length]);
            if (!codec_encode.encode(
                pixels.get(),
                pixels_length,
                pixels_encoded.get(),
                pixels_encoded_length
            )) {
                fprintf(stderr, "Failed to encode frame %zu/%zu for sample '%s'.\n", index_frame, sample_frames[index_sample], sample_names[index_sample].c_str());
                return 1;
            }

            // Add frames to stream.
            streams.back().frames.push_back({pixels_encoded.get(), pixels_encoded_length});
            frames_data.push_back(std::move(pixels_encoded));
        }

        // Generate the new avi data.
        std::vector<unsigned char> avi_data;
        if (!video_encode.compose(&avih, streams, avi_data)) {
            std::fprintf(stderr, "Failed to encode generated avi of sample '%s'.\n", sample_names[index_sample].c_str());
            return 1;
        }

        // Decode the new avi file.
        avi video_decode;
        if (!video_decode.parse(avi_data.data(), avi_data.size())) {
            std::fprintf(stderr, "Failed to parse generated avi of sample '%s'.\n", sample_names[index_sample].c_str());
            return 1;
        }

        // Select stream.
        unsigned int stream_number = 0xFFFFFFFF;
        for (size_t i = 0; i < video_decode.get_streams(); ++i) {
            const avi::stream_type& stream = video_decode.get_stream(i);
            if (
                (stream.strh->type == avi::fourcc("vids")) &&
                ((stream.strh->handler == avi::fourcc("hfyu")) || (stream.strh->handler == avi::fourcc("HFYU")))
            ) {
                if (
                    (stream.strf_vids != nullptr) &&
                    (stream.strf_vids->compression_identifier == avi::fourcc("HFYU"))
                ) {
                    stream_number = i;
                    break;
                }
            }
        }
        if (stream_number == 0xFFFFFFFF) {
            std::fprintf(stderr, "Failed find a HFYU encoded video stream inside the generated avi file.\n");
            return 1;
        }

        // Check the number of frames is the same.
        if (video_decode.get_stream(stream_number).frames.size() != sample_frames[index_sample]) {
            fprintf(stderr, "Failed to parse generated avi of sample '%s', number of frames does not match.\n", sample_names[index_sample].c_str());
            return 1;
        }

        // Setup decode codec.
        huffyuv codec_decode(reinterpret_cast<const unsigned char*>(video_decode.get_stream(stream_number).strf_vids), video_decode.get_stream(stream_number).strf_vids->header_size);
        if (!codec_decode.is_valid()) {
            fprintf(stderr, "Failed setup huffyuv decoder for sample '%s'.\n", sample_names[index_sample].c_str());
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

            if ((width != static_cast<size_t>(video_decode.get_stream(stream_number).strf_vids->width)) || (height != static_cast<size_t>(video_decode.get_stream(stream_number).strf_vids->height))) {
                fprintf(stderr, "Failed read frame %zu/%zu for sample '%s', frame dimensions do not match.\n", index_frame, sample_frames[index_sample], sample_names[index_sample].c_str());
                return 1;
            }

            unsigned long long int pixels_decoded_length = codec_decode.get_decoded_image_size();
            std::unique_ptr<unsigned char[]> pixels_decoded = std::unique_ptr<unsigned char[]>(new unsigned char[pixels_decoded_length]);
            if (!codec_decode.decode(
                video_decode.get_stream(stream_number).frames[index_frame].data,
                video_decode.get_stream(stream_number).frames[index_frame].length,
                pixels_decoded.get(),
                pixels_decoded_length
            )) {
                fprintf(stderr, "Failed to decode frame %zu/%zu for sample '%s'.\n", index_frame, sample_frames[index_sample], sample_names[index_sample].c_str());
                return 1;
            }

            switch (codec_decode.get_image_format()) {
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
                            if (codec_decode.get_image_format() == huffyuv::format_type::yuyv) {
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
