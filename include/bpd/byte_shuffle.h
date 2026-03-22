#pragma once
// Byte-plane shuffle and unshuffle.
// Reorders multi-byte records so that all bytes at position p across all
// records are contiguous, enabling more efficient entropy coding.

#include <cstdint>
#include <cstddef>

namespace bpd {

// Byte-plane shuffle: reorder bytes by position within records of width w.
// Input:  [A0 A1 A2 A3 B0 B1 B2 B3] (two 4-byte records)
// Output: [A0 B0] [A1 B1] [A2 B2] [A3 B3] (4 planes of 2 bytes each)
// Trailing bytes (len % width) are copied unchanged.
inline void byte_shuffle(const uint8_t* in, uint8_t* out, size_t len, int width) {
    size_t num_elements = len / width;
    size_t aligned = num_elements * width;

    for (int plane = 0; plane < width; ++plane) {
        for (size_t i = 0; i < num_elements; ++i) {
            out[plane * num_elements + i] = in[i * width + plane];
        }
    }

    // Copy trailing bytes
    for (size_t i = aligned; i < len; ++i) {
        out[num_elements * width + (i - aligned)] = in[i];
    }
}

// Inverse of byte_shuffle: restore original byte order.
inline void byte_unshuffle(const uint8_t* in, uint8_t* out, size_t len, int width) {
    size_t num_elements = len / width;
    size_t aligned = num_elements * width;

    for (int plane = 0; plane < width; ++plane) {
        for (size_t i = 0; i < num_elements; ++i) {
            out[i * width + plane] = in[plane * num_elements + i];
        }
    }

    for (size_t i = aligned; i < len; ++i) {
        out[i] = in[num_elements * width + (i - aligned)];
    }
}

} // namespace bpd
