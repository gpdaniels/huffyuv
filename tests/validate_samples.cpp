#include <huffyuv.hpp>

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
                fprintf(stderr, "Failed read frame %zu for sample '%s'.\n", index_frame, sample_names[index_sample].c_str());
                return 1;
            }
        }
        {
            size_t width = 0;
            size_t height = 0;
            std::unique_ptr<unsigned char[]> pixels_loaded = load_frame(index_sample, sample_frames[index_sample], width, height);
            if ((pixels_loaded != nullptr) || (width != 0) || (height != 0)) {
                fprintf(stderr, "Failed to fail reading frame %zu beyond end of sample '%s'.\n", sample_frames[index_sample], sample_names[index_sample].c_str());
                return 1;
            }
        }
    }

    // Validate avi files.
    for (size_t index_sample = 0; index_sample < sample_names.size(); ++index_sample) {
        size_t length = 0;
        std::unique_ptr<unsigned char[]> file = load_video(index_sample, length);
        huffyuv video;
        if ((file != nullptr) && (length != 0) && (!video.parse(file.get(), length))) {
            fprintf(stderr, "Failed to parse avi of sample '%s'.\n", sample_names[index_sample].c_str());
            return 1;
        }
    }

    return 0;
}
