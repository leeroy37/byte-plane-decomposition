// acf_analyze: Compare brute-force (Algorithm 1) vs autocorrelation (Algorithm 2)
// width detection, and display autocorrelation profiles.
// Usage: acf_analyze <file> [--block-size N]

#include <bpd/entropy.h>
#include <bpd/byte_shuffle.h>
#include <bpd/width_detect.h>
#include <bpd/width_detect_acf.h>

#include <iostream>
#include <fstream>
#include <vector>
#include <iomanip>
#include <string>

int main(int argc, char* argv[]) {
    if (argc < 2) {
        std::cerr << "Usage: acf_analyze <file> [--block-size N]\n";
        return 1;
    }

    std::string path = argv[1];
    size_t block_size = 65536;

    for (int i = 2; i < argc; i += 2) {
        if (std::string(argv[i]) == "--block-size" && i + 1 < argc) {
            block_size = std::stoul(argv[i + 1]);
        }
    }

    std::ifstream file(path, std::ios::binary);
    if (!file) { std::cerr << "Cannot open: " << path << "\n"; return 1; }

    file.seekg(0, std::ios::end);
    size_t file_size = file.tellg();
    file.seekg(0);

    std::vector<uint8_t> data(file_size);
    file.read(reinterpret_cast<char*>(data.data()), file_size);

    size_t num_blocks = file_size / block_size;
    if (num_blocks == 0) { std::cerr << "File too small\n"; return 1; }
    size_t max_blocks = std::min(num_blocks, size_t(32));

    std::cout << "File: " << path << " (" << file_size << " bytes)\n";
    std::cout << "Block size: " << block_size << " bytes, " << max_blocks << " blocks\n\n";

    // Per-block comparison
    std::cout << std::left
        << std::setw(6) << "Block"
        << std::setw(10) << "Entropy"
        << std::setw(12) << "Alg1-Width"
        << std::setw(12) << "Alg2-Width"
        << std::setw(12) << "Alg2-Cands"
        << std::setw(8) << "Match"
        << "\n" << std::string(60, '-') << "\n";

    int match_count = 0;
    int total_blocks = 0;

    for (size_t b = 0; b < max_blocks; ++b) {
        const uint8_t* block = data.data() + b * block_size;
        double block_entropy = bpd::shannon_entropy(block, block_size);

        int w1 = bpd::detect_width(block, block_size);
        int w2 = bpd::detect_width_acf(block, block_size);

        // Count candidates from autocorrelation (before fallback)
        auto candidates = bpd::detect_candidate_widths_acf(block, block_size);
        // +fallback widths not already in candidates
        size_t total_cands = candidates.size();
        for (uint32_t w : {2u, 4u, 8u}) {
            bool found = false;
            for (uint32_t c : candidates) {
                if (c == w) { found = true; break; }
            }
            if (!found) total_cands++;
        }

        bool match = (w1 == w2);
        if (match) match_count++;
        total_blocks++;

        std::cout << std::left
            << std::setw(6) << b
            << std::fixed << std::setprecision(2)
            << std::setw(10) << block_entropy
            << std::setw(12) << (w1 > 0 ? std::to_string(w1) : "---")
            << std::setw(12) << (w2 > 0 ? std::to_string(w2) : "---")
            << std::setw(12) << total_cands
            << std::setw(8) << (match ? "YES" : "NO")
            << "\n";
    }

    std::cout << "\nAgreement: " << match_count << "/" << total_blocks
        << " blocks (" << (100.0 * match_count / total_blocks) << "%)\n";
    std::cout << "Algorithm 1 evaluates 15 fixed widths per block\n";

    // Autocorrelation profile for block 0
    std::cout << "\n--- Autocorrelation profile (block 0) ---\n";
    auto profile = bpd::autocorrelation_profile(data.data(), block_size);

    if (profile.empty()) {
        std::cout << "(no data)\n";
    } else {
        // Find max for normalization in display
        double max_corr = 0.0;
        for (const auto& [lag, corr] : profile) {
            max_corr = std::max(max_corr, corr);
        }

        std::cout << std::left
            << std::setw(6) << "Lag"
            << std::setw(12) << "Correlation"
            << "Bar\n"
            << std::string(60, '-') << "\n";

        for (const auto& [lag, corr] : profile) {
            int bar_len = (max_corr > 0) ? static_cast<int>(40.0 * corr / max_corr) : 0;
            std::cout << std::left
                << std::setw(6) << lag
                << std::fixed << std::setprecision(4)
                << std::setw(12) << corr
                << std::string(bar_len, '#')
                << "\n";
        }
    }

    return 0;
}
