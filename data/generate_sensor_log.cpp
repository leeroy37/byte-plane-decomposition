// Deterministic synthetic sensor log generator.
// Produces 16 MB of 8-byte records: uint32 timestamp + int16 temp + int16 humidity.
// Seed=42, delta=1000, sigma=0.3. Output is byte-identical across platforms.

#include <iostream>
#include <fstream>
#include <random>
#include <cstdint>
#include <cstring>
#include <filesystem>

int main(int argc, char* argv[]) {
    const char* output = (argc > 1) ? argv[1] : "data/sensor_log.bin";
    size_t size = 16 * 1024 * 1024; // 16 MB

    auto parent = std::filesystem::path(output).parent_path();
    if (!parent.empty())
        std::filesystem::create_directories(parent);

    std::ofstream out(output, std::ios::binary);
    if (!out) { std::cerr << "Cannot create: " << output << "\n"; return 1; }

    std::mt19937 gen(42);
    std::normal_distribution<float> noise(0.0f, 0.3f);

    uint32_t timestamp = 1000000;
    int16_t temp = 2200;  // 22.00 C
    int16_t hum = 5500;   // 55.00%

    uint8_t record[8];
    size_t written = 0;

    while (written + 7 < size) {
        std::memcpy(record, &timestamp, 4);
        std::memcpy(record + 4, &temp, 2);
        std::memcpy(record + 6, &hum, 2);
        out.write(reinterpret_cast<const char*>(record), 8);

        timestamp += 1000;
        temp += static_cast<int16_t>(noise(gen));
        hum += static_cast<int16_t>(noise(gen));
        written += 8;
    }

    std::cout << "Generated " << written << " bytes (" << written / 8 << " records)\n";
    return 0;
}
