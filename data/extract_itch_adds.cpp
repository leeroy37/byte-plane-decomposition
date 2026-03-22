// Extract NASDAQ ITCH 5.0 Add Order messages (type 'A', 36 bytes) from raw ITCH stream.
// Input: decompressed ITCH binary file (2-byte big-endian length prefix per message)
// Output: flat binary file of concatenated 36-byte Add Order records.

#include <iostream>
#include <fstream>
#include <cstdint>
#include <vector>

int main(int argc, char* argv[]) {
    const char* input = (argc > 1) ? argv[1] : "data/nasdaq/itch_raw.bin";
    const char* output = (argc > 2) ? argv[2] : "data/nasdaq_itch_adds.bin";

    std::ifstream in(input, std::ios::binary);
    std::ofstream out(output, std::ios::binary);
    if (!in || !out) { std::cerr << "Cannot open files\n"; return 1; }

    uint64_t total_msgs = 0, add_orders = 0;
    std::vector<uint8_t> buf(256);

    while (in) {
        uint8_t len_bytes[2];
        if (!in.read(reinterpret_cast<char*>(len_bytes), 2)) break;
        uint16_t msg_len = (static_cast<uint16_t>(len_bytes[0]) << 8) | len_bytes[1];

        if (msg_len == 0 || msg_len > 200) {
            in.seekg(-1, std::ios::cur);
            continue;
        }

        if (buf.size() < msg_len) buf.resize(msg_len);
        if (!in.read(reinterpret_cast<char*>(buf.data()), msg_len)) break;

        total_msgs++;

        // 'A' = Add Order (no MPID), exactly 36 bytes
        if (static_cast<char>(buf[0]) == 'A' && msg_len == 36) {
            out.write(reinterpret_cast<const char*>(buf.data()), 36);
            add_orders++;
        }

        if (total_msgs % 1000000 == 0) {
            std::cerr << "\r" << total_msgs / 1000000 << "M messages, "
                      << add_orders << " add orders" << std::flush;
        }
    }

    std::cout << "\nTotal: " << total_msgs << " messages, "
              << add_orders << " Add Orders ("
              << add_orders * 36 << " bytes)\n";
    return 0;
}
