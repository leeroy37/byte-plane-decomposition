#pragma once
// Shannon entropy computation for byte sequences.
// Part of the byte-plane decomposition paper's Algorithm 1.

#include <cstdint>
#include <cstddef>
#include <cmath>

namespace bpd {

// Compute Shannon entropy of a byte sequence in bits per byte.
// Returns value in [0.0, 8.0]. 0.0 = constant, 8.0 = uniform random.
inline double shannon_entropy(const uint8_t* data, size_t len) {
    if (len == 0) return 0.0;

    uint32_t freq[256] = {0};
    for (size_t i = 0; i < len; ++i) {
        freq[data[i]]++;
    }

    double entropy = 0.0;
    double n = static_cast<double>(len);

    for (uint32_t count : freq) {
        if (count > 0) {
            double p = count / n;
            entropy -= p * std::log2(p);
        }
    }

    return entropy;
}

} // namespace bpd
