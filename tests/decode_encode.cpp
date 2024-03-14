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
        for (size_t i = 0; i < video.get_streams(); ++i) {
            const avi::stream_type& stream = video.get_stream(i);
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
            std::fprintf(stderr, "Failed find a HFYU encoded video stream inside the avi file.\n");
            return 1;
        }

        // Check the number of frames is the same.
        if (video.get_stream(stream_number).frames.size() != sample_frames[index_sample]) {
            fprintf(stderr, "Failed to parse avi of sample '%s', number of frames does not match.\n", sample_names[index_sample].c_str());
            return 1;
        }

        // Setup decode codec.
        huffyuv codec_decode(reinterpret_cast<const unsigned char*>(video.get_stream(stream_number).strf_vids), video.get_stream(stream_number).strf_vids->header_size);
        if (!codec_decode.is_valid()) {
            fprintf(stderr, "Failed setup huffyuv decoder for sample '%s'.\n", sample_names[index_sample].c_str());
            return 1;
        }

        // Setup encode codec.
        huffyuv codec_encode(
            codec_decode.get_image_width(),
            codec_decode.get_image_height(),
            codec_decode.is_interlaced(),
            codec_decode.is_decorrelated(),
            codec_decode.get_image_format(),
            codec_decode.get_image_predictor()
        );
        if (!codec_encode.is_valid()) {
            fprintf(stderr, "Failed setup huffyuv encoder for sample '%s'.\n", sample_names[index_sample].c_str());
            return 1;
        }

        // To support custom tables they'd need to be copied from the input to the output encoders.
        // codec_encode.tables[0] = codec_decode.tables[0];
        // codec_encode.tables[1] = codec_decode.tables[1];
        // codec_encode.tables[2] = codec_decode.tables[2];

        // Check that all the ffmpeg decoded frames match.
        for (size_t index_frame = 0; index_frame < sample_frames[index_sample]; ++index_frame) {

            unsigned long long int pixels_decoded_length = codec_decode.get_decoded_image_size();
            std::unique_ptr<unsigned char[]> pixels_decoded = std::unique_ptr<unsigned char[]>(new unsigned char[pixels_decoded_length]);
            if (!codec_decode.decode(
                video.get_stream(stream_number).frames[index_frame].data,
                video.get_stream(stream_number).frames[index_frame].length,
                pixels_decoded.get(),
                pixels_decoded_length
            )) {
                fprintf(stderr, "Failed to decode frame %zu/%zu for sample '%s'.\n", index_frame, sample_frames[index_sample], sample_names[index_sample].c_str());
                return 1;
            }

            unsigned long long int pixels_encoded_length = codec_decode.get_decoded_image_size();
            std::unique_ptr<unsigned char[]> pixels_encoded = std::unique_ptr<unsigned char[]>(new unsigned char[pixels_encoded_length]);
            if (!codec_encode.encode(
                pixels_decoded.get(),
                pixels_decoded_length,
                pixels_encoded.get(),
                pixels_encoded_length
            )) {
                fprintf(stderr, "Failed to encode frame %zu/%zu for sample '%s'.\n", index_frame, sample_frames[index_sample], sample_names[index_sample].c_str());
                return 1;
            }

            if (pixels_encoded_length != video.get_stream(stream_number).frames[index_frame].length) {
                fprintf(stderr, "Failed to match size of encoded frame %zu for sample '%s'.\n", index_frame, sample_names[index_sample].c_str());
                return 1;
            }

            for (size_t index_byte = 0; index_byte < pixels_encoded_length; ++index_byte) {
                if (pixels_encoded.get()[index_byte] != video.get_stream(stream_number).frames[index_frame].data[index_byte]) {
                    fprintf(stderr, "Failed to match frame %zu for sample '%s' at byte %zu.\n", index_frame, sample_names[index_sample].c_str(), index_byte);
                    return 1;
                }
            }
        }
    }
    
    return 0;
}
