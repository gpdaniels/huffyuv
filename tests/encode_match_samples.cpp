#include <huffyuv.hpp>

#include "samples.hpp"

int main(int argc, char* argv[]) {
    static_cast<void>(argc);
    static_cast<void>(argv);
    
    for (size_t index_sample = 0; index_sample < sample_names.size(); ++index_sample) {

        // TODO: Create/Prepare an avi file.
        //...

        // Add all the ffmpeg decoded frames.
        for (size_t index_frame = 0; index_frame < sample_frames[index_sample]; ++index_frame) {
            size_t width = 0;
            size_t height = 0;
            std::unique_ptr<unsigned char[]> pixels = load_frame(index_sample, index_frame, width, height);
            if ((pixels == nullptr) || (width == 0) || (height == 0)) {
                fprintf(stderr, "Failed read frame %zu/%zu for sample '%s'.\n", index_frame, sample_frames[index_sample], sample_names[index_sample].c_str());
                return 1;
            }

            // TODO: Frame encoding using the huffyuv class.
            //...
        }

        // TODO: Save the new avi file.
        //...

        // TODO: Compare the created file with the sample file.
        //...
    }
    
    return 0;
}
