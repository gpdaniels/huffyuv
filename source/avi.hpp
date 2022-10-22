#pragma once

#include <cstdio>
#include <vector>

class avi final {
private:
    // The files are comprised of a collection of chunks.
    struct chunk_type {
        // Four letter character code identifying the chunk type.
        unsigned int identifier;
        // Length of the chunk's data.
        unsigned int length;
    };
    static_assert(sizeof(chunk_type) == 8);

public:
    // The main avi header.
    struct avih_type final {
        // Contains the duration of one video frame in microseconds.
        unsigned int microseconds_per_frame;
        // Highest occuring data rate within the file.
        unsigned int max_bytes_per_seccond;
        // Pad to multiples of this size;
        unsigned int padding_granularity;
        // Flags:
        // 0x00000010 - The file has an index.
        // 0x00000020 - The order in which the video and audio chunks must be replayed is determined by the index and may differ from the order in which those chunks occur in the file.
        // 0x00000100 - The streams are properly interleaved into each other.
        // 0x00000800 - This flag indicates that the keyframe flags in the index are reliable.
        // 0x00010000 - The file was captured.
        // 0x00020000 - Is the content copyrighted.
        unsigned int flags;
        // Number of frames.
        unsigned int total_frames;
        // Number of frames audio data is skewed ahead of the video frames in interleaved files.
        unsigned int initial_frames;
        // Number of streams in the file.
        unsigned int stream_count;
        // Suggested buffer size for reading the file.
        unsigned int suggested_buffer_size;
        // Width of the video.
        unsigned int width;
        // Height of the video.
        unsigned int height;
        // Reserved.
        unsigned int reserved[4];
    };
    static_assert(sizeof(avih_type) == 56);

    // A stream header.
    struct strh_type {
        // Four letter character code identifying the stream type, can be 'vids', 'auds', 'mids' or 'txts' for video, audio and subtitles.
        unsigned int type;
        // Four letter character code identifying the codec to be used for this stream.
        unsigned int handler;
        // Flags:
        // 0x00000001 - Stream should not be activated by default.
        // 0x00010000 - Stream is a video stream using palettes where the palette is changing during playback.
        unsigned int flags;
        // Specifies priority of a stream. In a file with multiple streams of the same type, the highest priority might be the default.
        unsigned short priority;
        // Language of the stream.
        unsigned short language;
        // Number of the first block of the stream that is present in the file.
        unsigned int initial_frames;
        // rate / scale == samples / second
        unsigned int scale;
        // rate / scale == samples / second
        unsigned int rate;
        // Start time of stream.
        unsigned int start;
        // Size of stream in units as defined in rate and scale
        unsigned int length;
        // Size of buffer necessary to store blocks of that stream.
        unsigned int suggested_buffer_size;
        // Stream quality.
        unsigned int quality;
        // The number of bytes of the smallest part of a stream that should not be split any further.
        unsigned int sample_size;
        // The destination rectangle for a text or video stream within the width and height members of the main header.
        struct rect_type {
            short int left;
            short int top;
            short int right;
            short int bottom;
        } destination;
    };
    static_assert(sizeof(strh_type) == 56);

    // Stream formats are different for each stream type:
    // - BITMAPINFOHEADER for video streams.
    // - WAVEFORMATEX or PCMWAVEFORMAT for audio streams.
    // - Nothing for text streams.
    // - Nothing for midi streams.
    struct strf_vids_type {
        // Size of this BITMAPINFOHEADER and any additional data following it in bytes.
        unsigned int header_size;
        // Width of image in pixels.
        int width;
        // Height of image in pixels, for uncompressed images positive values mean bottom up and negative top down.
        int height;
        // The number of planes for the target device, should be 1.
        unsigned short planes;
        // Number of bits per pixel.
        unsigned short bit_count;
        // Flag or Four letter character code identifying the compression used.
        // 0x00000000 - Uncompressed RGB
        // 0x00000003 - Uncompressed RGB with color masks.
        // 0xXXXXXXXX - Four letter character code for compressed images.
        unsigned int compression_identifier;
        // Size of the image in bytes.
        unsigned int image_size;
        // Horizontal resolution in pixels per meter.
        int horizontal_pixels_per_meter;
        // Vertical resolution in pixels per meter.
        int vertical_pixels_per_meter;
        // The number of colour indices in the colour table that are actually used in the image.
        unsigned int colours_used;
        // The number of colour indices in the colour table that are important for displaying the image.
        unsigned int colours_important;
    };
    static_assert(sizeof(strf_vids_type) == 40);

    struct strf_auds_type {
        // TODO: Currently ignoring audio chunks.
    };
    static_assert(sizeof(strf_auds_type) == 1);

private:
    struct index_type {
        unsigned int chunk_id;
        // Flags:
        // 0x00000001 - The data of the chunk in the movi chunk is a rec list.
        // 0x00000010 - Frame is a keyframe.
        // 0x00000100 - The chunk does not affect the timing of the stream.
        unsigned int flags;
        unsigned int offset;
        unsigned int size;
    };
    static_assert(sizeof(index_type) == 16);

public:
    struct frame_type {
        const unsigned char* data;
        unsigned long long int length;
    };

private:
    struct chunk_node_type {
        const chunk_type* chunk;
        unsigned int form;
        std::vector<chunk_node_type> children;
    };

public:
    struct stream_type {
        const strh_type* strh;
        const strf_vids_type* strf_vids;
        const strf_auds_type* strf_auds;
        std::vector<frame_type> frames;
    };

private:
    chunk_node_type root_chunk_node;
    const avih_type* avih;
    std::vector<stream_type> streams;

public:
    bool parse(const unsigned char* data, unsigned long long int length) {
        this->streams.clear();

        if (!parse_chunks(&data[0], length, this->root_chunk_node)) {
            std::fprintf(stderr, "Error: Failed to parse root chunk.\n");
            return false;
        }

        if (this->root_chunk_node.chunk->identifier != fourcc("RIFF")) {
            std::fprintf(stderr, "Error: Root chunk is not a RIFF chunk.\n");
            return false;
        }

        if (!decode_avi_header()) {
            return false;
        }

        if (this->avih->stream_count > 255) {
            std::fprintf(stderr, "Error: Unsupported number of streams found in file.\n");
            return false;
        }

        if (!decode_stream_headers()) {
            return false;
        }

        if (this->avih->stream_count != this->streams.size()) {
            std::fprintf(stderr, "Error: Incorrect number of streams found in file.\n");
            return false;
        }

        if (!decode_streams()) {
            return false;
        }

        return true;
    }

    bool compose(
        const avih_type* avih,
        const std::vector<stream_type>& streams,
        std::vector<unsigned char>& video
    ) {
        constexpr static const auto dec_to_hex = [](int decimal, char* characters){
            constexpr const char* hex_characters = "0123456789ABCDEF";
            characters[0] = hex_characters[(decimal >> 4) & 0xF];
            characters[1] = hex_characters[(decimal >> 0) & 0xF];
        };

        video.clear();

        unsigned long long int index = 0;
        {
            unsigned int riff_size =
                4 +                                                     // CHUNK: RIFF FORM
                4 + 4 +                                                 // CHUNK: LIST[hdrl]
                4 +                                                     // CHUNK: LIST[hdrl] FORM
                4 + 4 +                                                 // CHUNK: avih
                sizeof(avih_type) +                                     // DATA:  avih_type
                4 + 4 +                                                 // CHUNK: LIST[strl]
                4;                                                      // CHUNK: LIST[strl] FORM
            for (size_t stream = 0; stream < streams.size(); ++stream) {
                riff_size +=
                    4 + 4 +                                             // CHUNK: strh
                    sizeof(strh_type) +                                 // DATA:  strh_type
                    4 + 4 +                                             // CHUNK: strf_vids
                    streams[stream].strf_vids->header_size +            // DATA:  strf_vids_type
                    streams[stream].strf_vids->header_size % 2;         // ALIGNMENT
            }
            riff_size +=
                4 + 4 +                                                 // CHUNK: movi
                4;                                                      // CHUNK: movi FORM
            for (size_t stream = 0; stream < streams.size(); ++stream) {
                for (size_t frame = 0; frame < streams[stream].frames.size(); ++frame) {
                    riff_size +=
                        4 + 4 +                                         // CHUNK: XXdc
                        streams[stream].frames[frame].length +          // DATA:  frame
                        streams[stream].frames[frame].length % 2;       // ALIGNMENT
                }
            }
            video.resize(8 + riff_size);
            copy_bytes("RIFF", &video[index], 4); index += 4;
            copy_bytes(&riff_size, &video[index], 4); index += 4;
            copy_bytes("AVI ", &video[index], 4); index += 4;

            {
                unsigned int hdrl_size =
                    4 +                                                 // CHUNK: LIST[hdrl] FORM
                    4 + 4 +                                             // CHUNK: avih
                    sizeof(avih_type) +                                 // DATA:  avih_type
                    4 + 4 +                                             // CHUNK: LIST[strl]
                    4;                                                  // CHUNK: LIST[strl] FORM
                for (size_t stream = 0; stream < streams.size(); ++stream) {
                    hdrl_size +=
                        4 + 4 +                                         // CHUNK: strh
                        sizeof(strh_type) +                             // DATA:  strh_type
                        4 + 4 +                                         // CHUNK: strf_vids
                        streams[stream].strf_vids->header_size +        // DATA:  strf_vids_type
                        streams[stream].strf_vids->header_size % 2;     // ALIGNMENT
                }
                copy_bytes("LIST", &video[index], 4); index += 4;
                copy_bytes(&hdrl_size, &video[index], 4); index += 4;
                copy_bytes("hdrl", &video[index], 4); index += 4;

                {
                    const unsigned int avih_size = sizeof(avih_type);
                    copy_bytes("avih", &video[index], 4); index += 4;
                    copy_bytes(&avih_size, &video[index], 4); index += 4;
                    copy_bytes(avih, &video[index], avih_size); index += avih_size;

                    unsigned int strl_size = 4;                         // CHUNK: LIST[strl] FORM
                    for (size_t stream = 0; stream < streams.size(); ++stream) {
                        strl_size +=
                            4 + 4 +                                     // CHUNK: strh
                            sizeof(strh_type) +                         // DATA:  strh_type
                            4 + 4 +                                     // CHUNK: strf_vids
                            streams[stream].strf_vids->header_size +    // DATA:  strf_vids_type
                            streams[stream].strf_vids->header_size % 2; // ALIGNMENT
                    }
                    copy_bytes("LIST", &video[index], 4); index += 4;
                    copy_bytes(&strl_size, &video[index], 4); index += 4;
                    copy_bytes("strl", &video[index], 4); index += 4;


                    for (size_t stream = 0; stream < streams.size(); ++stream) {
                        const strh_type* strh = streams[stream].strh;
                        const unsigned int strh_size = sizeof(strh_type);
                        copy_bytes("strh", &video[index], 4); index += 4;
                        copy_bytes(&strh_size, &video[index], 4); index += 4;
                        copy_bytes(strh, &video[index], strh_size); index += strh_size;

                        const strf_vids_type* strf_vids = streams[stream].strf_vids;
                        const unsigned int strf_vids_size = strf_vids->header_size;
                        copy_bytes("strf", &video[index], 4); index += 4;
                        copy_bytes(&strf_vids_size, &video[index], 4); index += 4;
                        copy_bytes(strf_vids, &video[index], strf_vids_size); index += strf_vids_size + (strf_vids_size % 2);
                    }
                }
            }

            {
                unsigned int movi_size = 4;                             // CHUNK: movi FORM
                for (size_t stream = 0; stream < streams.size(); ++stream) {
                    for (size_t frame = 0; frame < streams[stream].frames.size(); ++frame) {
                        movi_size +=
                            4 + 4 +                                     // CHUNK: XXdc
                            streams[stream].frames[frame].length +      // DATA:  frame
                            streams[stream].frames[frame].length % 2;   // ALIGNMENT
                    }
                }
                copy_bytes("LIST", &video[index], 4); index += 4;
                copy_bytes(&movi_size, &video[index], 4); index += 4;
                copy_bytes("movi", &video[index], 4); index += 4;

                for (size_t stream = 0; stream < streams.size(); ++stream) {
                    for (size_t frame = 0; frame < streams[stream].frames.size(); ++frame) {
                        char stream_id[4] = {'0', '0', 'd', 'c'};
                        dec_to_hex(stream, stream_id);
                        copy_bytes(stream_id, &video[index], 4); index += 4;
                        copy_bytes(&streams[stream].frames[frame].length, &video[index], 4); index += 4;
                        copy_bytes(streams[stream].frames[frame].data, &video[index], streams[stream].frames[frame].length); index += streams[stream].frames[frame].length + (streams[stream].frames[frame].length % 2);
                    }
                }
            }
        }

        return (video.size() == index);
    }

public:
    const avih_type* get_avih() const {
        return this->avih;
    }

    size_t get_streams() const {
        return this->streams.size();
    }

    const stream_type& get_stream(size_t stream_index) const {
        return this->streams[stream_index];
    }

    const std::vector<frame_type>& get_frames(size_t stream_index) const {
        return this->streams[stream_index].frames;
    }

public:
    constexpr static unsigned int fourcc(const char* data) {
        return  (static_cast<unsigned int>(data[3]) << 24) |
                (static_cast<unsigned int>(data[2]) << 16) |
                (static_cast<unsigned int>(data[1]) <<  8) |
                (static_cast<unsigned int>(data[0]) <<  0);
    }

private:
    constexpr static void copy_bytes(const void* source, void* destination, unsigned int length) {
        const unsigned char* data_source = static_cast<const unsigned char*>(source);
        unsigned char* data_destination = static_cast<unsigned char*>(destination);
        while (length--) {
            *data_destination++ = *data_source++;
        }
    }

private:
    static bool parse_chunks(const unsigned char* data, unsigned long long int length, chunk_node_type& node) {
        // Extract chunk header data.
        if (length < 8) {
            return false;
        }
        node.chunk = reinterpret_cast<const chunk_type*>(&data[0]);
        if (node.chunk->length + 8 > length) {
            std::fprintf(stderr, "Error: Chunk length is greater than remaining length.\n");
            return false;
        }
        const unsigned char* chunk_data = &data[8];
        // If chunk is a collection/list, recursively parse more.
        if ((node.chunk->identifier == fourcc("RIFF")) || (node.chunk->identifier == fourcc("LIST"))) {
            if ((node.chunk->length < 4) || (length < 12)) {
                std::fprintf(stderr, "Error: Chunk length is too short.\n");
                return false;
            }
            copy_bytes(&chunk_data[0], &node.form, 4);
            unsigned int index = 4;
            while ((index < node.chunk->length) && (index + 8 < length)) {
                node.children.push_back({});
                const chunk_node_type& child = node.children.back();
                if (!parse_chunks(&reinterpret_cast<const unsigned char*>(node.chunk)[sizeof(chunk_type) + index], node.chunk->length - 4, node.children.back())) {
                    std::fprintf(stderr, "Error: Failed to parse chunk.\n");
                    return false;
                }
                index += 8 + child.chunk->length + (child.chunk->length % 2);
            }
            return (index == node.chunk->length);
        }
        return true;
    }

    bool decode_avi_header() {
        // RIFF[AVI ]->LIST[hdrl]->avih

        // Start at the root chunk.
        const chunk_node_type* node = &this->root_chunk_node;

        // Validate.
        if (node->chunk->identifier != fourcc("RIFF")) {
            std::fprintf(stderr, "Error: Failed to decode avi header. First chunk is not 'RIFF'.\n");
            return false;
        }
        if (node->form != fourcc("AVI ")) {
            std::fprintf(stderr, "Error: Failed to decode avi header. 'RIFF' chunk is not of 'AVI ' form.\n");
            return false;
        }

        // Search for the LIST[hdrl] chunk.
        for (const chunk_node_type& child : node->children) {
            if (child.chunk->identifier == fourcc("LIST")) {
                if (child.form == fourcc("hdrl")) {
                    node = &child;
                    break;
                }
            }
        }

        // Validate.
        if ((node->chunk->identifier != fourcc("LIST")) || (node->form != fourcc("hdrl"))) {
            std::fprintf(stderr, "Error: Failed to decode avi header. 'RIFF[AVI ]' chunk does not contain a 'LIST[hdrl]' chunk.\n");
            return false;
        }

        // Search for the avih chunk.
        for (const chunk_node_type& child : node->children) {
            if (child.chunk->identifier == fourcc("avih")) {
                node = &child;
                break;
            }
        }

        if (node->chunk->identifier != fourcc("avih")) {
            std::fprintf(stderr, "Error: Failed to decode avi header. 'RIFF[AVI ]->LIST[hdrl]' chunk does not contain an 'avih' chunk.\n");
            return false;
        }

        if (node->chunk->length != sizeof(avih_type)) {
            std::fprintf(stderr, "Error: Failed to decode avi header. 'RIFF[AVI ]->LIST[hdrl]->avih' chunk is not the correct size.\n");
            return false;
        }

        this->avih = reinterpret_cast<const avih_type*>(&reinterpret_cast<const unsigned char*>(node->chunk)[sizeof(chunk_type)]);

        return true;
    }

    bool decode_stream_headers() {
        // RIFF[AVI ]->LIST[hdrl]->LIST[strl]->strh

        // Start at the root chunk.
        const chunk_node_type* node = &this->root_chunk_node;

        // Validate.
        if (node->chunk->identifier != fourcc("RIFF")) {
            std::fprintf(stderr, "Error: Failed to decode avi headers. First chunk is not 'RIFF'.\n");
            return false;
        }
        if (node->form != fourcc("AVI ")) {
            std::fprintf(stderr, "Error: Failed to decode avi headers. 'RIFF' chunk is not of 'AVI ' form.\n");
            return false;
        }

        // Search for the LIST[hdrl] chunk.
        for (const chunk_node_type& child : node->children) {
            if (child.chunk->identifier == fourcc("LIST")) {
                if (child.form == fourcc("hdrl")) {
                    node = &child;
                    break;
                }
            }
        }

        // Validate.
        if ((node->chunk->identifier != fourcc("LIST")) || (node->form != fourcc("hdrl"))) {
            std::fprintf(stderr, "Error: Failed to decode avi headers. 'RIFF[AVI ]' chunk does not contain a 'LIST[hdrl]' chunk.\n");
            return false;
        }

        // Process all the LIST[strl]->strh chunks.
        bool found_strl = false;
        for (const chunk_node_type& child : node->children) {
            if (child.chunk->identifier == fourcc("LIST")) {
                if (child.form == fourcc("strl")) {
                    found_strl = true;

                    const chunk_node_type* chunk_strl = &child;
                    std::vector<const strh_type*> strhs;
                    std::vector<const void*> strfs;
                    std::vector<const strf_auds_type*> strf_audss;
                    std::vector<const strf_vids_type*> strf_vidss;

                    // Search for the strh chunk.
                    bool found_strh = false;
                    bool found_strf = false;
                    for (const chunk_node_type& strl_child : chunk_strl->children) {
                        if (strl_child.chunk->identifier == fourcc("strh")) {
                            if (found_strh) {
                                std::fprintf(stderr, "Error: Failed to decode avi headers. 'RIFF[AVI ]->LIST[hdrl]->LIST[strl]' chunk contains multiple 'strh' chunks.\n");
                                return false;
                            }
                            found_strh = true;

                            if (strl_child.chunk->length != sizeof(strh_type)) {
                                std::fprintf(stderr, "Error: Failed to decode avi headers. 'RIFF[AVI ]->LIST[hdrl]->LIST[strl]->strh' chunk is not the correct size.\n");
                                return false;
                            }

                            strhs.push_back(reinterpret_cast<const strh_type*>(&reinterpret_cast<const unsigned char*>(strl_child.chunk)[sizeof(chunk_type)]));
                        }

                        if (strl_child.chunk->identifier == fourcc("strf")) {
                            if (found_strf) {
                                std::fprintf(stderr, "Error: Failed to decode avi headers. 'RIFF[AVI ]->LIST[hdrl]->LIST[strl]' chunk contains multiple 'strf' chunks.\n");
                                return false;
                            }
                            found_strf = true;

                            if (strhs.size() != strfs.size() + 1) {
                                std::fprintf(stderr, "Error: Failed to decode avi headers. 'RIFF[AVI ]->LIST[hdrl]->LIST[strl]->strf' chunk appeared before 'strh' chunk.\n");
                                return false;
                            }

                            if (strhs.back()->type == fourcc("vids")) {
                                if (strl_child.chunk->length < sizeof(strf_vids_type)) {
                                    std::fprintf(stderr, "Error: Failed to decode avi headers. 'RIFF[AVI ]->LIST[hdrl]->LIST[strl]->strf' chunk is not the correct size.\n");
                                    return false;
                                }

                                strfs.push_back(&reinterpret_cast<const unsigned char*>(strl_child.chunk)[sizeof(chunk_type)]);
                                strf_vidss.push_back(reinterpret_cast<const strf_vids_type*>(&reinterpret_cast<const unsigned char*>(strl_child.chunk)[sizeof(chunk_type)]));
                            }
                            else if (strhs.back()->type == fourcc("auds")) {
                                if (strl_child.chunk->length != sizeof(strf_auds_type)) {
                                    std::fprintf(stderr, "Error: Failed to decode avi headers. 'RIFF[AVI ]->LIST[hdrl]->LIST[strl]->strf' chunk is not the correct size.\n");
                                    return false;
                                }

                                strfs.push_back(&reinterpret_cast<const unsigned char*>(strl_child.chunk)[sizeof(chunk_type)]);
                                strf_audss.push_back(reinterpret_cast<const strf_auds_type*>(&reinterpret_cast<const unsigned char*>(strl_child.chunk)[sizeof(chunk_type)]));
                            }
                            else {
                                std::fprintf(stderr, "Warning: Unknown type of stream header %c%c%c%c.\n",
                                    reinterpret_cast<const unsigned char*>(&strhs.back()->type)[0],
                                    reinterpret_cast<const unsigned char*>(&strhs.back()->type)[1],
                                    reinterpret_cast<const unsigned char*>(&strhs.back()->type)[2],
                                    reinterpret_cast<const unsigned char*>(&strhs.back()->type)[3]
                                );
                            }
                        }
                    }

                    // Validate.
                    if (!found_strh) {
                        std::fprintf(stderr, "Error: Failed to decode avi headers. 'RIFF[AVI ]->LIST[hdrl]->LIST[strl]' chunk does not contain a 'strh' chunk.\n");
                        return false;
                    }

                    if (!found_strf) {
                        std::fprintf(stderr, "Error: Failed to decode avi headers. 'RIFF[AVI ]->LIST[hdrl]->LIST[strl]' chunk does not contain a 'strf' chunk.\n");
                        return false;
                    }

                    this->streams.push_back({});
                    this->streams.back().strh = strhs.back();
                    if (!strf_vidss.empty()) {
                        this->streams.back().strf_vids = strf_vidss.back();
                    }
                    if (!strf_audss.empty()) {
                        this->streams.back().strf_auds = strf_audss.back();
                    }
                }
            }
        }

        // Validate.
        if (!found_strl) {
            std::fprintf(stderr, "Error: Failed to decode avi headers. 'RIFF[AVI ]->LIST[hdrl]' chunk does not contain a 'LIST[strl]' chunk.\n");
            return false;
        }

        return true;
    }

    bool decode_streams() {
        // RIFF[AVI ]->idx1

        // Start at the root chunk.
        const chunk_node_type* node = &this->root_chunk_node;

        // Validate.
        if (node->chunk->identifier != fourcc("RIFF")) {
            std::fprintf(stderr, "Error: Failed to decode avi streams. First chunk is not 'RIFF'.\n");
            return false;
        }
        if (node->form != fourcc("AVI ")) {
            std::fprintf(stderr, "Error: Failed to decode avi streams. 'RIFF' chunk is not of 'AVI ' form.\n");
            return false;
        }

        // Search for the LIST[movi] chunk.
        for (const chunk_node_type& child : node->children) {
            if (child.chunk->identifier == fourcc("LIST")) {
                if (child.form == fourcc("movi")) {
                    node = &child;
                    break;
                }
            }
        }

        // Validate.
        if ((node->chunk->identifier != fourcc("LIST")) || (node->form != fourcc("movi"))) {
            std::fprintf(stderr, "Error: Failed to decode avi headers. 'RIFF[AVI ]' chunk does not contain a 'LIST[movi]' chunk.\n");
            return false;
        }

        // Search for the idx1 chunk.
        const chunk_node_type* chunk_index = &this->root_chunk_node;
        for (const chunk_node_type& child : chunk_index->children) {
            if (child.chunk->identifier == fourcc("idx1")) {
                chunk_index = &child;
                break;
            }
        }

        // Handy conversion function for stream number.
        constexpr static const auto hex_to_dec = [](unsigned char character)->int{
            if (('0' <= character) && (character <= '9')) {
                return (character - '0');
            }
            if (('a' <= character) && (character <= 'f')) {
                return 10 + (character - 'a');
            }
            if (('A' <= character) && (character <= 'F')) {
                return 10 + (character - 'A');
            }
            return -1;
        };

        // Validate.
        if (chunk_index->chunk->identifier != fourcc("idx1")) {
            // No index found. Just load chunks in the order they come.

            // RIFF[AVI ]->LIST[movi]->frame
            // RIFF[AVI ]->LIST[movi]->LIST[rec ]->frame
            // Where frame is one of:
            // - XXdb Uncompressed video frame
            // - XXdc Compressed video frame
            // - XXpc Palette change
            // - XXwb Audio data
            // Where XX is the stream number/index.
            // Where XX starts at zero and is the same order as strh/strf headers are written.

            // Process all the LIST[movi] chunks.
            for (const chunk_node_type& child : node->children) {
                if (child.chunk->identifier == fourcc("LIST")) {
                    if (child.form == fourcc("rec ")) {
                        for (const chunk_node_type& chunk_frame : child.children) {
                            int stream_id = hex_to_dec((chunk_frame.chunk->identifier >> 8) & 0xFF) + hex_to_dec((chunk_frame.chunk->identifier >> 0) & 0xFF) * 16;
                            if ((stream_id < 0) || (stream_id >= static_cast<int>(this->avih->stream_count))) {
                                std::fprintf(stderr, "Error: Failed to decode avi streams. 'RIFF[AVI ]->LIST[movi]->LIST[rec ]' contains a chunk with an invalid stream number.\n");
                                return false;
                            }
                            frame_type frame;
                            frame.data = &reinterpret_cast<const unsigned char*>(chunk_frame.chunk)[sizeof(chunk_type)];
                            frame.length = chunk_frame.chunk->length;
                            this->streams[static_cast<size_t>(stream_id)].frames.push_back(frame);
                        }
                    }
                }
                else {
                    const chunk_node_type& chunk_frame = child;
                    int stream_id = hex_to_dec((chunk_frame.chunk->identifier >> 8) & 0xFF) + hex_to_dec((chunk_frame.chunk->identifier >> 0) & 0xFF) * 16;
                    if ((stream_id < 0) || (stream_id >= static_cast<int>(this->avih->stream_count))) {
                        std::fprintf(stderr, "Error: Failed to decode avi streams. 'RIFF[AVI ]->LIST[movi]' contains a chunk with an invalid stream number.\n");
                        return false;
                    }
                    frame_type frame;
                    frame.data = &reinterpret_cast<const unsigned char*>(chunk_frame.chunk)[sizeof(chunk_type)];
                    frame.length = chunk_frame.chunk->length;
                    this->streams[static_cast<size_t>(stream_id)].frames.push_back(frame);
                }
            }

            return true;
        }

        if (chunk_index->chunk->length % sizeof(index_type) != 0) {
            std::fprintf(stderr, "Error: Failed to decode avi streams. 'RIFF[AVI ]->idx1' chunk is not a valid size.\n");
            return false;
        }

        for (unsigned int i = 0; i < chunk_index->chunk->length; i += sizeof(index_type)) {
            index_type index;
            copy_bytes(&reinterpret_cast<const unsigned char*>(chunk_index->chunk)[sizeof(chunk_type) + i], &index, sizeof(index_type));

            if (index.offset + index.size + 8 > node->chunk->length) {
                std::fprintf(stderr, "Error: Failed to decode avi streams. 'RIFF[AVI ]->idx1' chunk index offset is not a valid size.\n");
                return false;
            }

            // Check if the index is pointing at a rec list.
            if (index.flags & 0x00000001) {
                chunk_node_type chunk_list;
                parse_chunks(&reinterpret_cast<const unsigned char*>(node->chunk)[sizeof(chunk_type) + index.offset], index.size + 8, chunk_list);
                if ((chunk_list.chunk->identifier != fourcc("LIST")) || (chunk_list.form != fourcc("rec "))) {
                    std::fprintf(stderr, "Error: Failed to decode avi streams. 'RIFF[AVI ]->idx1' chunk index offset does not contain a 'LIST[rec ]' chunk.\n");
                    return false;
                }
                for (const chunk_node_type& chunk_frame : chunk_list.children) {
                    int stream_id = hex_to_dec((chunk_frame.chunk->identifier >> 8) & 0xFF) + hex_to_dec((chunk_frame.chunk->identifier >> 0) & 0xFF) * 16;
                    if ((stream_id < 0) || (stream_id >= static_cast<int>(this->avih->stream_count))) {
                        std::fprintf(stderr, "Error: Failed to decode avi streams. 'RIFF[AVI ]->idx1' chunk index offset to 'LIST[rec ]' contains a chunk with an invalid stream number.\n");
                        return false;
                    }
                    frame_type frame;
                    frame.data = &reinterpret_cast<const unsigned char*>(chunk_frame.chunk)[sizeof(chunk_type)];
                    frame.length = chunk_frame.chunk->length;
                    this->streams[static_cast<size_t>(stream_id)].frames.push_back(frame);
                }
                return true;
            }
            else {
                int stream_id = hex_to_dec((index.chunk_id >> 8) & 0xFF) + hex_to_dec((index.chunk_id >> 0) & 0xFF) * 16;
                if ((stream_id < 0) || (stream_id >= static_cast<int>(this->avih->stream_count))) {
                    std::fprintf(stderr, "Error: Failed to decode avi streams. 'RIFF[AVI ]->LIST[movi]' contains a chunk with an invalid stream number.\n");
                    return false;
                }
                frame_type frame;
                frame.data = &reinterpret_cast<const unsigned char*>(node->chunk)[sizeof(chunk_type) + index.offset + sizeof(chunk_type)];
                frame.length = index.size;
                this->streams[static_cast<size_t>(stream_id)].frames.push_back(frame);
            }
        }

        return true;
    }
};
