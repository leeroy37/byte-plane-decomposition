// gap_measure: Measure the universality gap between shuffle+delta+ZSTD and plain ZSTD.
// Usage: gap_measure <file> [--block-size N]

#include <bpd/entropy.h>
#include <bpd/byte_shuffle.h>
#include <bpd/delta.h>
#include <bpd/width_detect.h>

#include <iostream>
#include <fstream>
#include <vector>
#include <iomanip>
#include <string>
#include <algorithm>
#include <zstd.h>

int main(int argc, char* argv[]) {
    if (argc < 2) {
        std::cerr << "Usage: gap_measure <file> [--block-size N]\n";
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

    size_t total_plain = 0, total_shuffle = 0;

    for (size_t b = 0; b < num_blocks; ++b) {
        const uint8_t* block = data.data() + b * block_size;

        // Plain ZSTD
        std::vector<uint8_t> zbuf(ZSTD_compressBound(block_size));
        size_t plain_size = ZSTD_compress(zbuf.data(), zbuf.size(), block, block_size, 3);
        if (ZSTD_isError(plain_size)) { plain_size = block_size; }
        total_plain += plain_size;

        // Shuffle + delta + ZSTD at detected width
        int width = bpd::detect_width(block, block_size);
        if (width > 0) {
            size_t num_elem = block_size / width;
            std::vector<uint8_t> shuffled(block_size);
            bpd::byte_shuffle(block, shuffled.data(), block_size, width);

            for (int p = 0; p < width; ++p) {
                bpd::delta_encode(shuffled.data() + p * num_elem, num_elem);
            }

            std::vector<uint8_t> sbuf(ZSTD_compressBound(block_size));
            size_t shuf_size = ZSTD_compress(sbuf.data(), sbuf.size(),
                shuffled.data(), block_size, 3);
            if (ZSTD_isError(shuf_size)) { shuf_size = block_size; }
            total_shuffle += std::min(plain_size, shuf_size);
        } else {
            total_shuffle += plain_size;
        }
    }

    double r_plain = file_size / static_cast<double>(total_plain);
    double r_shuffle = file_size / static_cast<double>(total_shuffle);
    double gap = r_shuffle / r_plain;

    std::cout << std::fixed << std::setprecision(2);
    std::cout << "File:           " << path << "\n";
    std::cout << "Original:       " << file_size << " bytes\n";
    std::cout << "Plain ZSTD:     " << total_plain << " bytes (ratio " << r_plain << ":1)\n";
    std::cout << "Shuffle+ZSTD:   " << total_shuffle << " bytes (ratio " << r_shuffle << ":1)\n";
    std::cout << "Gap G:          " << gap << "x\n";

    return 0;
}
