#pragma once
// Per-plane delta encoding and decoding.
// Applied after byte-plane shuffle to exploit temporal correlation.
// delta[i] = data[i] - data[i-1] (wrapping uint8 arithmetic).

#include <cstdint>
#include <cstddef>

namespace bpd {

// In-place delta encoding. First byte is preserved as anchor.
inline void delta_encode(uint8_t* data, size_t len) {
    for (size_t i = len - 1; i > 0; --i) {
        data[i] = data[i] - data[i - 1];
    }
}

// In-place delta decoding (inverse of delta_encode).
inline void delta_decode(uint8_t* data, size_t len) {
    for (size_t i = 1; i < len; ++i) {
        data[i] = data[i] + data[i - 1];
    }
}

} // namespace bpd
