#pragma once

class huffyuv final {
public:
    enum class predictor {
        left, plane, median
    };

public:
    ~huffyuv() {
        //?
    }

    huffyuv() {
        //?
    }

public:
    void encode_start(unsigned char* data_encoded, int length_encoded) {
        //?
    }

    void encode_frame(unsigned char* data, int length, unsigned char* data_encoded, int length_encoded) {
        //?
    }

    void encode_finish(unsigned char encoded, int length_encoded) {
        //?
    }

public:
    void decode_start(unsigned char* data_decoded, int length_decoded) {
        //?
    }

    void decode_frame(unsigned char* data, int length, unsigned char* data_decoded, int length_decoded) {
        //?
    }

    void decode_finish(unsigned char* data_decoded, int length_decoded) {
        //?
    }
};
