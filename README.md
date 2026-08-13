# byte-plane-decomposition

[![DOI](https://zenodo.org/badge/DOI/10.5281/zenodo.21924012.svg)](https://doi.org/10.5281/zenodo.21924012)

Schema-free detection of optimal byte-plane decomposition width for lossless compression, with universality gap measurements across structured binary data.

Implementation for the paper **"Entropy-Minimized Byte-Plane Decomposition: Auto-Detection and the Universality Gap in Lossless Compression."**

Byte-plane decomposition — reordering serialized multi-byte data by byte position before compression — is widely deployed (HDF5, Blosc, Parquet) but every existing implementation requires the user to specify the field width. This work presents an algorithm that detects the optimal width automatically by minimizing average per-plane Shannon entropy, and characterizes when and why it helps across five datasets with universality gaps ranging from 1.00× to 40.8×.

---

## Prerequisites

| Requirement | Version |
|-------------|---------|
| C++ compiler | GCC 11+, Clang 14+, or MSVC 2022 (C++20) |
| CMake | 3.20+ |
| Disk space | ~2 GB for all datasets |

zstd v1.5.7 is fetched and built from source automatically by CMake (`FetchContent`) — no system library install needed, but the first configure requires network access.

**Ubuntu/Debian:**
```bash
sudo apt install build-essential cmake
```

**macOS (Homebrew):**
```bash
brew install cmake
```

---

## Build

### With CMakePresets (recommended)

```bash
cmake --preset release
cmake --build --preset release
```

Binaries are written to `build/`.

### Manual

```bash
cmake -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build
```

---

## Dataset Setup

### 1. Synthetic sensor log (generated, ~16 MB)

```bash
./build/generate_sensor_log data/sensor_log.bin
```

Deterministic output (seed=42). No download needed.

### 2. Silesia Corpus (~67 MB)

```bash
bash data/download_silesia.sh
```

Downloads and extracts to `data/silesia/`. Used for `mr`, `sao`, and false-positive text files.

### 3. Intel Berkeley Lab sensor data (~50 MB)

Download the raw text file from <http://db.csail.mit.edu/labdata/labdata.html>, then convert:

```bash
./build/convert_intel_lab data.txt.gz data/intel_lab.bin
```

### 4. NASDAQ ITCH 5.0 (~125 MB extracted)

Download a sample file from <https://emi.nasdaq.com/ITCH/Nasdaq%20ITCH/>, then extract Add Order records:

```bash
./build/extract_itch_adds input.itch data/nasdaq_itch_adds.bin
```

---

## Run Experiments

Reproduce all paper tables (builds required, at least sensor_log and Silesia needed):

```bash
cd experiments && bash run_all.sh
```

### Verify paper numbers

Checks all 38 numeric claims in the paper against tool output:

```bash
bash experiments/verify_paper.sh
```

Expected output:
```
Results: 38 passed, 0 failed, 0 skipped
ALL CHECKS PASSED
```

Files that are not present are automatically skipped (SKIP counts up instead of FAIL).

---

## Repository Layout

```
include/bpd/          Header-only algorithm library
  entropy.h           Shannon entropy computation
  byte_shuffle.h      Byte-plane decomposition / unshuffle
  delta.h             Per-plane delta encode/decode
  width_detect.h      Algorithm 1 — fixed candidate set
  width_detect_acf.h  Algorithm 2 — byte-match frequency

tools/
  bpd_analyze.cpp     Width detection + entropy analysis
  acf_analyze.cpp     Algorithm 2 analyzer
  gap_measure.cpp     Universality gap measurement (requires zstd)

data/
  download_silesia.sh         Download Silesia corpus
  generate_sensor_log.cpp     Synthetic dataset generator
  convert_intel_lab.cpp       Intel Lab text → binary
  extract_itch_adds.cpp       NASDAQ ITCH → binary

experiments/
  run_all.sh          Reproduce all paper tables
  verify_paper.sh     Verify 38 paper numbers

paper/
  paper.md            Full paper source
```

---

## Citation

See [CITATION.cff](CITATION.cff) or cite as:

> Steffen Goehring. Entropy-Minimized Byte-Plane Decomposition: Auto-Detection and the Universality Gap in Lossless Compression. Zenodo, 2026. https://doi.org/10.5281/zenodo.21924012
