#include "samples.hpp"

int main(int argc, char* argv[]) {
    static_cast<void>(argc);
    static_cast<void>(argv);

    for (size_t index_sample = 0; index_sample < sample_names.size(); ++index_sample) {
        {
            int width = 0;
            int height = 0;
            std::unique_ptr<unsigned char[]> pixels = load_frame(index_sample, 0, width, height);
            if ((pixels != nullptr) || (width != 0) || (height != 0)) {
                fprintf(stderr, "Failed to fail reading frame zero for sample '%s'.\n", sample_names[index_sample].c_str());
                return 1;
            }
        }
        for (size_t index_frame = 1; index_frame <= sample_frames[index_sample]; ++index_frame) {
            int width = 0;
            int height = 0;
            std::unique_ptr<unsigned char[]> pixels = load_frame(index_sample, index_frame, width, height);
            if ((pixels == nullptr) || (width == 0) || (height == 0)) {
                fprintf(stderr, "Failed read frame %zu for sample '%s'.\n", sample_frames[index_sample], sample_names[index_sample].c_str());
                return 1;
            }
        }
        {
            int width = 0;
            int height = 0;
            std::unique_ptr<unsigned char[]> pixels = load_frame(index_sample, sample_frames[index_sample] + 1, width, height);
            if ((pixels != nullptr) || (width != 0) || (height != 0)) {
                fprintf(stderr, "Failed to fail reading frame %zu for sample '%s'.\n", sample_frames[index_sample], sample_names[index_sample].c_str());
                return 1;
            }
        }
    }
    
    return 0;
}
