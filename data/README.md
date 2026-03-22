# Dataset Download Instructions

All datasets used in the paper. Binary data files are NOT committed to the repo.

## Synthetic sensor log (generated)

```bash
./build/generate_sensor_log data/sensor_log.bin
```

Produces 16 MB of 8-byte records (uint32 timestamp + int16 temp + int16 humidity).
Deterministic: seed=42, identical output on any platform.

## Silesia Corpus (7 MB sao + 9.5 MB mr)

```bash
./data/download_silesia.sh
```

Downloads the full 12-file corpus (~67 MB). Key files: `sao` (28-byte star catalog records), `mr` (16-bit MRI).

## Intel Berkeley Lab Sensors (50.8 MB binary)

1. Download: `curl -L -o data/intel_lab/data.txt.gz http://db.csail.mit.edu/labdata/data.txt.gz`
2. Decompress: `gunzip data/intel_lab/data.txt.gz`
3. Convert: `./build/convert_intel_lab data/intel_lab/data.txt data/intel_lab.bin`

Produces 24-byte records: uint32 epoch + uint32 moteid + 4*float32 (temp/humidity/light/voltage).

## NASDAQ ITCH 5.0 (125 MB binary after extraction)

1. Download BX exchange file: `curl -L -o data/nasdaq/itch.gz https://emi.nasdaq.com/ITCH/Nasdaq%20ITCH/01302020.NASDAQ_ITCH50.gz`
2. Decompress: `gunzip data/nasdaq/itch.gz`
3. Extract Add Orders: `./build/extract_itch_adds data/nasdaq/itch data/nasdaq_itch_adds.bin`

Produces 36-byte Add Order records. Protocol spec: https://www.nasdaqtrader.com/content/technicalSupport/specifications/dataproducts/NQTVITCHSpecification.pdf
