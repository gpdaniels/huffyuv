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
        // Pointer to the chunk's data, note that this includes the form bytes for RIFF and LIST chunks.
        const unsigned char* data;
        // RIFF and LIST chunks hold a four letter character code identifying their form as the first four bytes in their data.
        unsigned int form;
        // Extracted child chunks within RIFF and LIST chunks.
        std::vector<chunk_type> children;
    };

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
        unsigned short  planes;
        // Number of bits per pixel.
        unsigned short  bit_count;
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
        // Optional data.
        std::vector<unsigned char> extradata;
    };
    static_assert(sizeof(strf_vids_type) == 40 + sizeof(std::vector<unsigned char>));

    struct strf_auds_type {
        // TODO: Currently ignoring audio chunks.
    };
    struct strf_type {
        unsigned int identifier;
        strf_vids_type strf_vids;
        strf_auds_type strf_auds;
    };

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
        std::vector<unsigned char> data;
    };

private:
    chunk_type root_chunk;
    avih_type avih;
    std::vector<strh_type> strhs;
    std::vector<strf_type> strfs;
    std::vector<std::vector<frame_type>> frames;

public:
    bool parse(const unsigned char* data, unsigned long long int length) {
        this->strhs.clear();
        this->strfs.clear();
        this->frames.clear();

        if (!parse_chunks(&data[0], length, this->root_chunk)) {
            std::fprintf(stderr, "Error: Failed to parse root chunk.\n");
            return false;
        }

        if (this->root_chunk.identifier != fourcc("RIFF")) {
            std::fprintf(stderr, "Error: Root chunk is not a RIFF chunk.\n");
            return false;
        }

        if (!decode_avi_header()) {
            return false;
        }

        if (this->avih.stream_count > 255) {
            std::fprintf(stderr, "Error: Unsupported number of streams found in file.\n");
            return false;
        }

        if (!decode_stream_headers()) {
            return false;
        }

        if ((this->avih.stream_count != this->strhs.size()) || (this->avih.stream_count != this->strfs.size())) {
            std::fprintf(stderr, "Error: Incorrect number of streams found in file.\n");
            return false;
        }

        if (!decode_streams()) {
            return false;
        }

        return true;
    }

public:
    const avih_type& get_avih() const {
        return this->avih;
    }

    const std::vector<strh_type>& get_strhs() const {
        return this->strhs;
    }

    const std::vector<strf_type>& get_strfs() const {
        return this->strfs;
    }

    const std::vector<std::vector<frame_type>>& get_frames() const {
        return this->frames;
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
    static bool parse_chunks(const unsigned char* data, unsigned long long int length, chunk_type& chunk) {
        // Extract chunk header data.
        if (length < 8) {
            return false;
        }
        copy_bytes(&data[0], &chunk.identifier, 4);
        copy_bytes(&data[4], &chunk.length, 4);
        if (chunk.length + 8 > length) {
            std::fprintf(stderr, "Error: Chunk length is greater than remaining length.\n");
            return false;
        }
        chunk.data = &data[8];
        // If chunk is a collection/list, recursively parse more.
        if ((chunk.identifier == fourcc("RIFF")) || (chunk.identifier == fourcc("LIST"))) {
            if ((chunk.length < 4) || (length < 12)) {
                std::fprintf(stderr, "Error: Chunk length is too short.\n");
                return false;
            }
            copy_bytes(&chunk.data[0], &chunk.form, 4);
            unsigned int index = 4;
            while ((index < chunk.length) && (index + 8 < length)) {
                chunk_type child = {};
                if (!parse_chunks(&chunk.data[index], chunk.length - 4, child)) {
                    std::fprintf(stderr, "Error: Failed to parse chunk.\n");
                    return false;
                }
                index += 8 + child.length + (child.length % 2);
                chunk.children.push_back(std::move(child));
            }
            return (index == chunk.length);
        }
        return true;
    }

    bool decode_avi_header() {
        // RIFF[AVI ]->LIST[hdrl]->avih

        // Start at the root chunk.
        const chunk_type* chunk = &this->root_chunk;

        // Validate.
        if (chunk->identifier != fourcc("RIFF")) {
            std::fprintf(stderr, "Error: Failed to decode avi header. First chunk is not 'RIFF'.\n");
            return false;
        }
        if (chunk->form != fourcc("AVI ")) {
            std::fprintf(stderr, "Error: Failed to decode avi header. 'RIFF' chunk is not of 'AVI ' form.\n");
            return false;
        }

        // Search for the LIST[hdrl] chunk.
        for (const chunk_type& child : chunk->children) {
            if (child.identifier == fourcc("LIST")) {
                if (child.form == fourcc("hdrl")) {
                    chunk = &child;
                    break;
                }
            }
        }

        // Validate.
        if ((chunk->identifier != fourcc("LIST")) || (chunk->form != fourcc("hdrl"))) {
            std::fprintf(stderr, "Error: Failed to decode avi header. 'RIFF[AVI ]' chunk does not contain a 'LIST[hdrl]' chunk.\n");
            return false;
        }

        // Search for the avih chunk.
        for (const chunk_type& child : chunk->children) {
            if (child.identifier == fourcc("avih")) {
                chunk = &child;
                break;
            }
        }

        if (chunk->identifier != fourcc("avih")) {
            std::fprintf(stderr, "Error: Failed to decode avi header. 'RIFF[AVI ]->LIST[hdrl]' chunk does not contain an 'avih' chunk.\n");
            return false;
        }

        if (chunk->length != sizeof(avih_type)) {
            std::fprintf(stderr, "Error: Failed to decode avi header. 'RIFF[AVI ]->LIST[hdrl]->avih' chunk is not the correct size.\n");
            return false;
        }

        copy_bytes(&chunk->data[0],  &this->avih.microseconds_per_frame, 4);
        copy_bytes(&chunk->data[4],  &this->avih.max_bytes_per_seccond, 4);
        copy_bytes(&chunk->data[8],  &this->avih.padding_granularity, 4);
        copy_bytes(&chunk->data[12], &this->avih.flags, 4);
        copy_bytes(&chunk->data[16], &this->avih.total_frames, 4);
        copy_bytes(&chunk->data[20], &this->avih.initial_frames, 4);
        copy_bytes(&chunk->data[24], &this->avih.stream_count, 4);
        copy_bytes(&chunk->data[28], &this->avih.suggested_buffer_size, 4);
        copy_bytes(&chunk->data[32], &this->avih.width, 4);
        copy_bytes(&chunk->data[36], &this->avih.height, 4);
        copy_bytes(&chunk->data[40], &this->avih.reserved[0], 4);
        copy_bytes(&chunk->data[44], &this->avih.reserved[1], 4);
        copy_bytes(&chunk->data[48], &this->avih.reserved[2], 4);
        copy_bytes(&chunk->data[52], &this->avih.reserved[3], 4);

        return true;
    }

    bool decode_stream_headers() {
        // RIFF[AVI ]->LIST[hdrl]->LIST[strl]->strh

        // Start at the root chunk.
        const chunk_type* chunk = &this->root_chunk;

        // Validate.
        if (chunk->identifier != fourcc("RIFF")) {
            std::fprintf(stderr, "Error: Failed to decode avi headers. First chunk is not 'RIFF'.\n");
            return false;
        }
        if (chunk->form != fourcc("AVI ")) {
            std::fprintf(stderr, "Error: Failed to decode avi headers. 'RIFF' chunk is not of 'AVI ' form.\n");
            return false;
        }

        // Search for the LIST[hdrl] chunk.
        for (const chunk_type& child : chunk->children) {
            if (child.identifier == fourcc("LIST")) {
                if (child.form == fourcc("hdrl")) {
                    chunk = &child;
                    break;
                }
            }
        }

        // Validate.
        if ((chunk->identifier != fourcc("LIST")) || (chunk->form != fourcc("hdrl"))) {
            std::fprintf(stderr, "Error: Failed to decode avi headers. 'RIFF[AVI ]' chunk does not contain a 'LIST[hdrl]' chunk.\n");
            return false;
        }

        // Process all the LIST[strl]->strh chunks.
        bool found_strl = false;
        for (const chunk_type& child : chunk->children) {
            if (child.identifier == fourcc("LIST")) {
                if (child.form == fourcc("strl")) {
                    found_strl = true;

                    const chunk_type* chunk_strl = &child;

                    // Search for the strh chunk.
                    bool found_strh = false;
                    bool found_strf = false;
                    for (const chunk_type& strl_child : chunk_strl->children) {
                        if (strl_child.identifier == fourcc("strh")) {
                            if (found_strh) {
                                std::fprintf(stderr, "Error: Failed to decode avi headers. 'RIFF[AVI ]->LIST[hdrl]->LIST[strl]' chunk contains multiple 'strh' chunks.\n");
                                return false;
                            }
                            found_strh = true;

                            if (strl_child.length != sizeof(strh_type)) {
                                std::fprintf(stderr, "Error: Failed to decode avi headers. 'RIFF[AVI ]->LIST[hdrl]->LIST[strl]->strh' chunk is not the correct size.\n");
                                return false;
                            }

                            strh_type strh = {};
                            copy_bytes(&strl_child.data[0],  &strh.type, 4);
                            copy_bytes(&strl_child.data[4],  &strh.handler, 4);
                            copy_bytes(&strl_child.data[8],  &strh.flags, 4);
                            copy_bytes(&strl_child.data[12], &strh.priority, 2);
                            copy_bytes(&strl_child.data[14], &strh.language, 2);
                            copy_bytes(&strl_child.data[16], &strh.initial_frames, 4);
                            copy_bytes(&strl_child.data[20], &strh.scale, 4);
                            copy_bytes(&strl_child.data[24], &strh.rate, 4);
                            copy_bytes(&strl_child.data[28], &strh.start, 4);
                            copy_bytes(&strl_child.data[32], &strh.length, 4);
                            copy_bytes(&strl_child.data[36], &strh.suggested_buffer_size, 4);
                            copy_bytes(&strl_child.data[40], &strh.quality, 4);
                            copy_bytes(&strl_child.data[44], &strh.sample_size, 4);
                            copy_bytes(&strl_child.data[48], &strh.destination.left, 2);
                            copy_bytes(&strl_child.data[50], &strh.destination.top, 2);
                            copy_bytes(&strl_child.data[52], &strh.destination.right, 2);
                            copy_bytes(&strl_child.data[54], &strh.destination.bottom, 2);
                            this->strhs.push_back(strh);
                        }

                        if (strl_child.identifier == fourcc("strf")) {
                            if (found_strf) {
                                std::fprintf(stderr, "Error: Failed to decode avi headers. 'RIFF[AVI ]->LIST[hdrl]->LIST[strl]' chunk contains multiple 'strf' chunks.\n");
                                return false;
                            }
                            found_strf = true;

                            if (this->strhs.size() != this->strfs.size() + 1) {
                                std::fprintf(stderr, "Error: Failed to decode avi headers. 'RIFF[AVI ]->LIST[hdrl]->LIST[strl]->strf' chunk appeared before 'strh' chunk.\n");
                                return false;
                            }

                            if (this->strhs.back().type == fourcc("vids")) {
                                // Ensure the strf_vids_type is the expected size (40), given that we've added an extradata member into it.
                                static_assert((sizeof(strf_vids_type) - sizeof(decltype(strf_vids_type::extradata))) == 40);

                                if (strl_child.length < 40) {
                                    std::fprintf(stderr, "Error: Failed to decode avi headers. 'RIFF[AVI ]->LIST[hdrl]->LIST[strl]->strf' chunk is not the correct size.\n");
                                    return false;
                                }

                                strf_vids_type strf_vids;
                                copy_bytes(&strl_child.data[0], &strf_vids.header_size, 4);
                                copy_bytes(&strl_child.data[4], &strf_vids.width, 4);
                                copy_bytes(&strl_child.data[8], &strf_vids.height, 4);
                                copy_bytes(&strl_child.data[12], &strf_vids.planes, 2);
                                copy_bytes(&strl_child.data[14], &strf_vids.bit_count, 2);
                                copy_bytes(&strl_child.data[16], &strf_vids.compression_identifier, 4);
                                copy_bytes(&strl_child.data[20], &strf_vids.image_size, 4);
                                copy_bytes(&strl_child.data[24], &strf_vids.horizontal_pixels_per_meter, 4);
                                copy_bytes(&strl_child.data[28], &strf_vids.vertical_pixels_per_meter, 4);
                                copy_bytes(&strl_child.data[32], &strf_vids.colours_used, 4);
                                copy_bytes(&strl_child.data[36], &strf_vids.colours_important, 4);

                                unsigned int extradata_size = strl_child.length - 40;
                                if (extradata_size > 0) {
                                    strf_vids.extradata.resize(extradata_size);
                                    copy_bytes(&strl_child.data[40], strf_vids.extradata.data(), extradata_size);
                                }

                                strf_type strf = {};
                                strf.identifier = fourcc("vids");
                                strf.strf_vids = strf_vids;
                                this->strfs.push_back(strf);
                            }

                            if (this->strhs.back().type == fourcc("auds")) {
                                if (strl_child.length != sizeof(strf_auds_type)) {
                                    std::fprintf(stderr, "Error: Failed to decode avi headers. 'RIFF[AVI ]->LIST[hdrl]->LIST[strl]->strf' chunk is not the correct size.\n");
                                    return false;
                                }

                                strf_auds_type strf_auds;
                                // TODO: Currently ignoring audio chunks.

                                strf_type strf;
                                strf.identifier = fourcc("auds");
                                strf.strf_auds = strf_auds;
                                this->strfs.push_back(strf);
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
        const chunk_type* chunk = &this->root_chunk;

        // Validate.
        if (chunk->identifier != fourcc("RIFF")) {
            std::fprintf(stderr, "Error: Failed to decode avi streams. First chunk is not 'RIFF'.\n");
            return false;
        }
        if (chunk->form != fourcc("AVI ")) {
            std::fprintf(stderr, "Error: Failed to decode avi streams. 'RIFF' chunk is not of 'AVI ' form.\n");
            return false;
        }

        // Search for the LIST[movi] chunk.
        for (const chunk_type& child : chunk->children) {
            if (child.identifier == fourcc("LIST")) {
                if (child.form == fourcc("movi")) {
                    chunk = &child;
                    break;
                }
            }
        }

        // Validate.
        if ((chunk->identifier != fourcc("LIST")) || (chunk->form != fourcc("movi"))) {
            std::fprintf(stderr, "Error: Failed to decode avi headers. 'RIFF[AVI ]' chunk does not contain a 'LIST[movi]' chunk.\n");
            return false;
        }

        // Search for the idx1 chunk.
        const chunk_type* chunk_index = &this->root_chunk;
        for (const chunk_type& child : chunk_index->children) {
            if (child.identifier == fourcc("idx1")) {
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

        // Allocate the streams in the frames.
        this->frames.resize(this->avih.stream_count);

        // Validate.
        if (chunk_index->identifier != fourcc("idx1")) {
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
            for (const chunk_type& child : chunk->children) {
                if (child.identifier == fourcc("LIST")) {
                    if (child.form == fourcc("rec ")) {
                        for (const chunk_type& chunk_frame : child.children) {
                            int stream_id = hex_to_dec((chunk_frame.identifier >> 8) & 0xFF) + hex_to_dec((chunk_frame.identifier >> 0) & 0xFF) * 16;
                            if ((stream_id < 0) || (stream_id >= static_cast<int>(this->avih.stream_count))) {
                                std::fprintf(stderr, "Error: Failed to decode avi streams. 'RIFF[AVI ]->LIST[movi]->LIST[rec ]' contains a chunk with an invalid stream number.\n");
                                return false;
                            }
                            frame_type frame;
                            frame.data.resize(chunk_frame.length);
                            copy_bytes(&chunk_frame.data[0], frame.data.data(), chunk_frame.length);
                            this->frames[static_cast<size_t>(stream_id)].push_back(frame);
                        }
                    }
                }
                else {
                    const chunk_type& chunk_frame = child;
                    int stream_id = hex_to_dec((chunk_frame.identifier >> 8) & 0xFF) + hex_to_dec((chunk_frame.identifier >> 0) & 0xFF) * 16;
                    if ((stream_id < 0) || (stream_id >= static_cast<int>(this->avih.stream_count))) {
                        std::fprintf(stderr, "Error: Failed to decode avi streams. 'RIFF[AVI ]->LIST[movi]' contains a chunk with an invalid stream number.\n");
                        return false;
                    }
                    frame_type frame;
                    frame.data.resize(chunk_frame.length);
                    copy_bytes(&chunk_frame.data[0], frame.data.data(), chunk_frame.length);
                    this->frames[static_cast<size_t>(stream_id)].push_back(frame);
                }
            }

            return true;
        }

        if (chunk_index->length % sizeof(index_type) != 0) {
            std::fprintf(stderr, "Error: Failed to decode avi streams. 'RIFF[AVI ]->idx1' chunk is not a valid size.\n");
            return false;
        }

        for (unsigned int i = 0; i < chunk_index->length; i += sizeof(index_type)) {
            index_type index;
            copy_bytes(&chunk_index->data[i], &index, sizeof(index_type));

            if (index.offset + index.size + 8 > chunk->length) {
                std::fprintf(stderr, "Error: Failed to decode avi streams. 'RIFF[AVI ]->idx1' chunk index offset is not a valid size.\n");
                return false;
            }

            // Check if the index is pointing at a rec list.
            if (index.flags & 0x00000001) {
                chunk_type chunk_list;
                parse_chunks(&chunk->data[index.offset], index.size + 8, chunk_list);
                if ((chunk_list.identifier != fourcc("LIST")) || (chunk_list.form != fourcc("rec "))) {
                    std::fprintf(stderr, "Error: Failed to decode avi streams. 'RIFF[AVI ]->idx1' chunk index offset does not contain a 'LIST[rec ]' chunk.\n");
                    return false;
                }
                for (const chunk_type& chunk_frame : chunk_list.children) {
                    int stream_id = hex_to_dec((chunk_frame.identifier >> 8) & 0xFF) + hex_to_dec((chunk_frame.identifier >> 0) & 0xFF) * 16;
                    if ((stream_id < 0) || (stream_id >= static_cast<int>(this->avih.stream_count))) {
                        std::fprintf(stderr, "Error: Failed to decode avi streams. 'RIFF[AVI ]->idx1' chunk index offset to 'LIST[rec ]' contains a chunk with an invalid stream number.\n");
                        return false;
                    }
                    frame_type frame;
                    frame.data.resize(chunk_frame.length);
                    copy_bytes(&chunk_frame.data[0], frame.data.data(), chunk_frame.length);
                    this->frames[static_cast<size_t>(stream_id)].push_back(frame);
                }
                return true;
            }
            else {
                int stream_id = hex_to_dec((index.chunk_id >> 8) & 0xFF) + hex_to_dec((index.chunk_id >> 0) & 0xFF) * 16;
                if ((stream_id < 0) || (stream_id >= static_cast<int>(this->avih.stream_count))) {
                    std::fprintf(stderr, "Error: Failed to decode avi streams. 'RIFF[AVI ]->LIST[movi]' contains a chunk with an invalid stream number.\n");
                    return false;
                }
                frame_type frame;
                frame.data.resize(index.size);
                copy_bytes(&chunk->data[index.offset + 8], frame.data.data(), index.size);
                this->frames[static_cast<size_t>(stream_id)].push_back(frame);
            }
        }

        return true;
    }
};
