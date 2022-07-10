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
            int width = 0;
            int height = 0;
            std::unique_ptr<unsigned char[]> pixels = load_frame(index_sample, index_frame, width, height);

            // TODO: Frame encoding using the huffyuv class.
            //...
        }

        std::string sample_video = "samples/" + sample_names[index_sample];

        // TODO: Compare the created file with the sample file.
        //...
    }
    
    return 0;
}
