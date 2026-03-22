// Convert Intel Berkeley Lab sensor CSV to 24-byte binary records.
// Input: data.txt from http://db.csail.mit.edu/labdata/data.txt.gz
// Output: 24-byte records: uint32 epoch + uint32 moteid + 4*float32

#include <iostream>
#include <fstream>
#include <sstream>
#include <cstdint>
#include <cstring>
#include <string>

struct Record {
    uint32_t epoch;
    uint32_t moteid;
    float temperature;
    float humidity;
    float light;
    float voltage;
};
static_assert(sizeof(Record) == 24);

int main(int argc, char* argv[]) {
    const char* input = (argc > 1) ? argv[1] : "data/intel_lab/data.txt";
    const char* output = (argc > 2) ? argv[2] : "data/intel_lab.bin";

    std::ifstream in(input);
    std::ofstream out(output, std::ios::binary);
    if (!in || !out) { std::cerr << "Cannot open files\n"; return 1; }

    std::string line;
    uint32_t count = 0, skipped = 0;

    while (std::getline(in, line)) {
        std::istringstream iss(line);
        std::string date, time;
        uint32_t epoch, moteid;
        float temp, humidity, light, voltage;

        if (!(iss >> date >> time >> epoch >> moteid >> temp >> humidity >> light >> voltage)) {
            skipped++;
            continue;
        }

        Record r{epoch, moteid, temp, humidity, light, voltage};
        out.write(reinterpret_cast<const char*>(&r), sizeof(r));
        count++;
    }

    std::cout << "Converted " << count << " records (" << count * 24 << " bytes)"
              << ", skipped " << skipped << " malformed lines\n";
    return 0;
}
