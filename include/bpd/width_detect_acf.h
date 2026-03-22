#pragma once
// Algorithm 2: Autocorrelation-based byte-plane width detection.
// Replaces brute-force candidate set with sampled autocorrelation to find
// periodic byte repetitions, then validates top candidates via entropy.
// Supports arbitrary widths up to max_width (default 64) without a predefined set.

#include "entropy.h"
#include "byte_shuffle.h"

#include <cstdint>
#include <cstddef>
#include <vector>
#include <algorithm>
#include <utility>
#include <cmath>

namespace bpd {

// Phase 1: Sampled autocorrelation candidate detection.
// Samples ~4096 positions and counts exact byte matches at each lag.
// Returns up to top_k candidate widths sorted by autocorrelation strength.
inline std::vector<uint32_t> detect_candidate_widths_acf(
    const uint8_t* data, size_t len, uint32_t max_width = 64, size_t top_k = 5)
{
    if (len < max_width + 1) return {};

    size_t sample_step = std::max(size_t(1), len / 4096);

    // Compute autocorrelation for each lag
    std::vector<double> autocorr(max_width + 1, 0.0);
    uint32_t sample_count = 0;

    for (size_t i = 0; i + max_width < len; i += sample_step) {
        for (uint32_t lag = 2; lag <= max_width; ++lag) {
            if (data[i] == data[i + lag]) {
                autocorr[lag] += 1.0;
            }
        }
        sample_count++;
    }

    if (sample_count == 0) return {};

    // Normalize to [0, 1]
    for (uint32_t lag = 2; lag <= max_width; ++lag) {
        autocorr[lag] /= sample_count;
    }

    // Find peak autocorrelation
    double max_corr = 0.0;
    for (uint32_t lag = 2; lag <= max_width; ++lag) {
        max_corr = std::max(max_corr, autocorr[lag]);
    }

    // Threshold: absolute floor + relative to peak
    double threshold = std::max(0.05, max_corr * 0.5);

    // Collect local maxima above threshold
    std::vector<std::pair<double, uint32_t>> peaks;
    for (uint32_t lag = 2; lag <= max_width; ++lag) {
        if (autocorr[lag] < threshold) continue;
        bool is_peak = true;
        if (lag > 2 && autocorr[lag - 1] > autocorr[lag]) is_peak = false;
        if (lag < max_width && autocorr[lag + 1] > autocorr[lag]) is_peak = false;
        if (is_peak) {
            peaks.push_back({autocorr[lag], lag});
        }
    }

    // Sort by correlation descending, take top_k
    std::sort(peaks.begin(), peaks.end(), [](const auto& a, const auto& b) {
        return a.first > b.first;
    });

    std::vector<uint32_t> candidates;
    for (size_t i = 0; i < std::min(peaks.size(), top_k); ++i) {
        candidates.push_back(peaks[i].second);
    }

    return candidates;
}

// Return the raw autocorrelation profile for analysis/plotting.
// Returns vector of (lag, normalized_correlation) for lags 2..max_width.
inline std::vector<std::pair<uint32_t, double>> autocorrelation_profile(
    const uint8_t* data, size_t len, uint32_t max_width = 64)
{
    std::vector<std::pair<uint32_t, double>> profile;
    if (len < max_width + 1) return profile;

    size_t sample_step = std::max(size_t(1), len / 4096);
    std::vector<double> autocorr(max_width + 1, 0.0);
    uint32_t sample_count = 0;

    for (size_t i = 0; i + max_width < len; i += sample_step) {
        for (uint32_t lag = 2; lag <= max_width; ++lag) {
            if (data[i] == data[i + lag]) {
                autocorr[lag] += 1.0;
            }
        }
        sample_count++;
    }

    if (sample_count == 0) return profile;

    for (uint32_t lag = 2; lag <= max_width; ++lag) {
        profile.push_back({lag, autocorr[lag] / sample_count});
    }

    return profile;
}

// Algorithm 2: Full autocorrelation-based width detection.
// Phase 1: Sampled autocorrelation → top-5 candidate widths
// Phase 2: Add fallback widths {2, 4, 8}
// Phase 3: Validate each candidate via entropy minimization (same as Algorithm 1)
//
// Returns w* > 0 if a width reduces average plane entropy by at least (1-tau),
// 0 if no width passes the threshold.
inline int detect_width_acf(const uint8_t* data, size_t len,
                            double tau = 0.80, uint32_t max_width = 64)
{
    if (len < 32) return 0;

    double base_entropy = shannon_entropy(data, len);
    if (base_entropy < 1.0) return 0;

    // Phase 1: Autocorrelation-based candidate detection
    auto candidates = detect_candidate_widths_acf(data, len, max_width);

    // Phase 2: Ensure standard widths are always tested as fallback
    for (uint32_t w : {2u, 4u, 8u}) {
        bool found = false;
        for (uint32_t c : candidates) {
            if (c == w) { found = true; break; }
        }
        if (!found) candidates.push_back(w);
    }

    // Phase 3: Validate each candidate with entropy check.
    // Collect all passing widths, then prefer fundamental (smallest divisor).
    struct Candidate {
        int width;
        double avg_entropy;
    };
    std::vector<Candidate> passing;
    std::vector<uint8_t> shuffled(len);

    for (uint32_t width : candidates) {
        if (len < static_cast<size_t>(width) * 4) continue;

        byte_shuffle(data, shuffled.data(), len, static_cast<int>(width));

        size_t num_elements = len / width;
        double total_entropy = 0.0;
        for (uint32_t plane = 0; plane < width; ++plane) {
            total_entropy += shannon_entropy(
                shuffled.data() + plane * num_elements, num_elements);
        }

        double avg_entropy = total_entropy / width;

        if (avg_entropy < base_entropy * tau) {
            passing.push_back({static_cast<int>(width), avg_entropy});
        }
    }

    if (passing.empty()) return 0;

    // Select best width, then check for harmonic suppression.
    // First, find the width with lowest average entropy (same as Algorithm 1).
    std::sort(passing.begin(), passing.end(), [](const Candidate& a, const Candidate& b) {
        return a.avg_entropy < b.avg_entropy;
    });

    int best_width = passing[0].width;
    double best_entropy = passing[0].avg_entropy;

    // Harmonic suppression: if a smaller width divides best_width and its
    // entropy is within 10% of best, prefer the fundamental period.
    // This prevents harmonics (e.g., 56 = 2×28) from being preferred when
    // the fundamental (28) already captures the structure.
    for (size_t i = 1; i < passing.size(); ++i) {
        if (passing[i].width < best_width &&
            best_width % passing[i].width == 0 &&
            passing[i].avg_entropy < best_entropy * 1.10) {
            best_width = passing[i].width;
            best_entropy = passing[i].avg_entropy;
        }
    }

    return best_width;
}

} // namespace bpd
