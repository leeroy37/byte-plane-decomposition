# Reproducing the Paper Results

## Prerequisites

- C++20 compiler (GCC 11+, Clang 14+, or MSVC 2022)
- CMake 3.20+ (fetches and builds zstd v1.5.7 from source automatically)
- ~2 GB disk space for datasets
- Internet connection for dataset downloads and the first CMake configure

## Quick Start

```bash
git clone https://github.com/user/byte-plane-decomposition.git
cd byte-plane-decomposition
cmake -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build

# Download datasets
./data/download_silesia.sh
./build/generate_sensor_log data/sensor_log.bin

# Run all experiments
cd experiments
./run_all.sh
```

Expected runtime: ~10 minutes. Compare output against files in `experiments/expected/`.

## Step by Step

### 1. Download datasets

```bash
# Silesia Corpus
./data/download_silesia.sh

# Synthetic sensor data
./build/generate_sensor_log data/sensor_log.bin

# Intel Berkeley Lab (optional, ~34 MB download)
mkdir -p data/intel_lab
curl -L -o data/intel_lab/data.txt.gz http://db.csail.mit.edu/labdata/data.txt.gz
gunzip data/intel_lab/data.txt.gz
./build/convert_intel_lab data/intel_lab/data.txt data/intel_lab.bin

# NASDAQ ITCH (optional, ~5 GB download)
mkdir -p data/nasdaq
curl -L -o data/nasdaq/itch.gz "https://emi.nasdaq.com/ITCH/Nasdaq%20ITCH/01302020.NASDAQ_ITCH50.gz"
gunzip data/nasdaq/itch.gz
./build/extract_itch_adds data/nasdaq/itch data/nasdaq_itch_adds.bin
```

### 2. Run experiments

Each script in `experiments/` corresponds to a table in the paper:

| Script | Paper Section |
|--------|---------------|
| `table_3_4_sao_widths.sh` | Table 3.4 (SAO width sweep) |
| `table_7_1_detection.sh` | Table 7.1 (width detection accuracy) |
| `table_7_2_gap.sh` | Table 7.2 (universality gap) |
| `tau_sensitivity.sh` | Section 3.5 (threshold sensitivity) |
| `block_size_sensitivity.sh` | Section 7.7 (block size sensitivity) |

### 3. Verify results

Small numerical differences (< 0.1%) are expected due to floating-point
entropy computation across platforms. The universality gap values and
detected widths should match exactly.
