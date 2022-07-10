#pragma once

#include <cstdio>
#include <vector>

struct chunk_type {
    unsigned int identifier;
    unsigned int length;
    const unsigned char* data;
    unsigned int form;
    std::vector<chunk_type> children;
};

class avi final {
private:
    chunk_type root_chunk;

public:
    bool parse(const unsigned char* data, unsigned int length) {
        if (!parse_chunks(&data[0], length, this->root_chunk)) {
            std::fprintf(stderr, "Error: Failed to parse root chunk.\n");
            return false;
        }
        if (this->root_chunk.identifier != as_int("RIFF")) {
            std::fprintf(stderr, "Error: Root chunk is not a RIFF chunk.\n");
            return false;
        }
        if (!decode_avi_header()) {
            return false;
        }
        if (!decode_stream_header()) {
            return false;
        }
        if (!decode_stream_format()) {
            return false;
        }
        return true;
    }

private:
    constexpr static unsigned int as_int(const char* data) {
        return  (static_cast<unsigned int>(data[3]) << 24) |
                (static_cast<unsigned int>(data[2]) << 16) |
                (static_cast<unsigned int>(data[1]) <<  8) |
                (static_cast<unsigned int>(data[0]) <<  0);
    }
    constexpr static unsigned int as_int(const unsigned char* data) {
        return  (static_cast<unsigned int>(data[3]) << 24) |
                (static_cast<unsigned int>(data[2]) << 16) |
                (static_cast<unsigned int>(data[1]) <<  8) |
                (static_cast<unsigned int>(data[0]) <<  0);
    }

    static bool parse_chunks(const unsigned char* data, unsigned int length, chunk_type& chunk) {
        // Extract chunk header data.
        if (length < 8) {
            return false;
        }
        chunk.identifier = as_int(&data[0]);
        chunk.length = as_int(&data[4]);
        if (chunk.length + 8 > length) {
            std::fprintf(stderr, "Error: Chunk length is greater than remaining length.\n");
            return false;
        }
        chunk.data = &data[8];
        // If chunk is a collection/list, recursively parse more.
        if ((chunk.identifier == as_int("RIFF")) || (chunk.identifier == as_int("LIST"))) {
            if ((chunk.length < 4) || (length < 12)) {
                std::fprintf(stderr, "Error: Chunk length is too short.\n");
                return false;
            }
            chunk.form = as_int(&chunk.data[0]);
            unsigned int index = 4;
            while ((index < chunk.length) && (index + 8 < length)) {
                chunk_type child = {};
                if (!parse_chunks(&chunk.data[index], chunk.length - 4, child)) {
                    std::fprintf(stderr, "Error: Failed to parse chunk.\n");
                    return false;
                }
                index += 8 + child.length + (child.length % 2);
                chunk.children.push_back(child);
            }
            return (index == chunk.length);
        }
        return true;
    }

    bool decode_avi_header() {
        // RIFF[AVI ]->LIST[hdrl]->avih
        // TODO: Go from root chunk to header.
        //...
        return true;
    }

    static bool decode_stream_header() {
        // RIFF[AVI ]->LIST[hdrl]->LIST[strl]->strh[vids]
        // TODO: Go from root chunk to header.
        //...
        return true;
    }

    static bool decode_stream_format() {
        // RIFF[AVI ]->LIST[hdrl]->LIST[strl]->strf[HFYU]
        // TODO: Go from root chunk to header.
        //...
        return true;
    }
};
