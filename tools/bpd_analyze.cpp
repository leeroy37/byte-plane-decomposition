// bpd_analyze: Per-file entropy analysis with width detection.
// Usage: bpd_analyze <file> [--block-size N] [--width W]

#include <bpd/entropy.h>
#include <bpd/byte_shuffle.h>
#include <bpd/delta.h>
#include <bpd/width_detect.h>

#include <iostream>
#include <fstream>
#include <vector>
#include <iomanip>
#include <cstring>
#include <string>

int main(int argc, char* argv[]) {
    if (argc < 2) {
        std::cerr << "Usage: bpd_analyze <file> [--block-size N] [--width W]\n";
        return 1;
    }

    std::string path = argv[1];
    size_t block_size = 65536;
    int force_width = -1;

    for (int i = 2; i < argc; i += 2) {
        if (std::string(argv[i]) == "--block-size" && i + 1 < argc) {
            block_size = std::stoul(argv[i + 1]);
        } else if (std::string(argv[i]) == "--width" && i + 1 < argc) {
            force_width = std::stoi(argv[i + 1]);
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

    std::cout << std::left
        << std::setw(6) << "Block"
        << std::setw(10) << "Entropy"
        << std::setw(10) << "Width"
        << std::setw(12) << "AvgPlaneH"
        << std::setw(12) << "AvgDeltaH"
        << std::setw(10) << "Reduction"
        << "\n" << std::string(60, '-') << "\n";

    for (size_t b = 0; b < max_blocks; ++b) {
        const uint8_t* block = data.data() + b * block_size;
        double block_entropy = bpd::shannon_entropy(block, block_size);

        int width = (force_width > 0) ? force_width
                                       : bpd::detect_width(block, block_size);

        if (width > 0 && block_size >= static_cast<size_t>(width) * 4) {
            size_t num_elem = block_size / width;
            std::vector<uint8_t> shuffled(block_size);
            bpd::byte_shuffle(block, shuffled.data(), block_size, width);

            // Compute per-plane entropy before and after delta
            double sum_raw = 0, sum_delta = 0;
            std::vector<uint8_t> delta_buf(shuffled);
            for (int p = 0; p < width; ++p) {
                sum_raw += bpd::shannon_entropy(shuffled.data() + p * num_elem, num_elem);
                bpd::delta_encode(delta_buf.data() + p * num_elem, num_elem);
                sum_delta += bpd::shannon_entropy(delta_buf.data() + p * num_elem, num_elem);
            }

            double avg_raw = sum_raw / width;
            double avg_delta = sum_delta / width;
            double reduction = (1.0 - avg_raw / block_entropy) * 100.0;

            std::cout << std::left
                << std::setw(6) << b
                << std::fixed << std::setprecision(2)
                << std::setw(10) << block_entropy
                << std::setw(10) << width
                << std::setw(12) << avg_raw
                << std::setw(12) << avg_delta
                << std::setw(10) << (std::to_string(static_cast<int>(reduction)) + "%")
                << "\n";
        } else {
            std::cout << std::left
                << std::setw(6) << b
                << std::fixed << std::setprecision(2)
                << std::setw(10) << block_entropy
                << std::setw(10) << "---"
                << std::setw(12) << "---"
                << std::setw(12) << "---"
                << std::setw(10) << "---"
                << "\n";
        }
    }

    // Per-plane detail for first block
    int w = (force_width > 0) ? force_width
                               : bpd::detect_width(data.data(), block_size);
    if (w > 0 && block_size >= static_cast<size_t>(w) * 4) {
        size_t num_elem = block_size / w;
        std::vector<uint8_t> shuffled(block_size);
        bpd::byte_shuffle(data.data(), shuffled.data(), block_size, w);

        std::vector<uint8_t> delta_buf(shuffled);
        for (int p = 0; p < w; ++p) {
            bpd::delta_encode(delta_buf.data() + p * num_elem, num_elem);
        }

        std::cout << "\nPer-plane entropy (block 0, width=" << w << "):\n";
        std::cout << std::left
            << std::setw(6) << "Plane"
            << std::setw(12) << "Entropy"
            << std::setw(12) << "AfterDelta"
            << "\n";
        for (int p = 0; p < w; ++p) {
            double h_raw = bpd::shannon_entropy(shuffled.data() + p * num_elem, num_elem);
            double h_delta = bpd::shannon_entropy(delta_buf.data() + p * num_elem, num_elem);
            std::cout << std::left
                << std::setw(6) << p
                << std::fixed << std::setprecision(3)
                << std::setw(12) << h_raw
                << std::setw(12) << h_delta
                << "\n";
        }
    }

    return 0;
}
