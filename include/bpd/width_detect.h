#pragma once
// Algorithm 1: Entropy-minimized byte-plane width detection.
// Finds the record width w* that minimizes average per-plane Shannon entropy.
// Returns 0 if no width reduces entropy by at least (1 - tau) × 100%.

#include "entropy.h"
#include "byte_shuffle.h"

#include <cstdint>
#include <cstddef>
#include <vector>
#include <algorithm>

namespace bpd {

// Default candidate widths: common struct sizes in systems programming.
// Includes 28 (SAO Star Catalog record width, discovered during experiments).
constexpr int DEFAULT_WIDTHS[] = {2, 3, 4, 5, 6, 7, 8, 10, 12, 14, 16, 20, 24, 28, 32};
constexpr int NUM_DEFAULT_WIDTHS = sizeof(DEFAULT_WIDTHS) / sizeof(DEFAULT_WIDTHS[0]);

// Detect optimal byte-plane decomposition width.
//
// Parameters:
//   data  - input byte block
//   len   - block length in bytes
//   tau   - acceptance threshold (default 0.80 = require 20% entropy reduction)
//
// Returns:
//   w* > 0 if a width reduces average plane entropy by at least (1-tau),
//   0 if no width passes the threshold.
inline int detect_width(const uint8_t* data, size_t len, double tau = 0.80) {
    if (len < 32) return 0;

    // Compute baseline entropy
    double base_entropy = shannon_entropy(data, len);

    // Already very low entropy — no shuffle needed
    if (base_entropy < 1.0) return 0;

    int best_width = 0;
    double best_entropy = base_entropy;

    std::vector<uint8_t> shuffled(len);

    for (int i = 0; i < NUM_DEFAULT_WIDTHS; ++i) {
        int w = DEFAULT_WIDTHS[i];
        if (len < static_cast<size_t>(w) * 4) continue; // need >= 4 elements

        byte_shuffle(data, shuffled.data(), len, w);

        // Compute average entropy across all byte planes
        size_t num_elements = len / w;
        double total_entropy = 0.0;

        for (int plane = 0; plane < w; ++plane) {
            total_entropy += shannon_entropy(
                shuffled.data() + plane * num_elements, num_elements);
        }

        double avg_entropy = total_entropy / w;

        // Must be significantly better (at least (1-tau) reduction)
        if (avg_entropy < best_entropy * tau) {
            best_entropy = avg_entropy;
            best_width = w;
        }
    }

    return best_width;
}

} // namespace bpd
