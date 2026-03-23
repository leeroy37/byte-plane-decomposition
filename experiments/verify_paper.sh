#!/bin/bash
# verify_paper.sh — Verify all paper numbers against tool output.
# Run from repo root: bash experiments/verify_paper.sh
# Exits non-zero if any check fails.
#
# Requires: built tools in build/Release/, dataset files at paths below.
# Dataset paths can be overridden via environment variables.
set -euo pipefail

# --- Configuration ---
BUILD="${BUILD:-build}"
SENSOR_LOG="${SENSOR_LOG:-data/sensor_log.bin}"
INTEL_LAB="${INTEL_LAB:-data/intel_lab.bin}"
MR="${MR:-data/silesia/mr}"
SAO="${SAO:-data/silesia/sao}"
NASDAQ="${NASDAQ:-data/nasdaq_itch_adds.bin}"
SILESIA="${SILESIA:-data/silesia}"

PASS=0
FAIL=0
SKIP=0

pass() { echo "  PASS: $1"; PASS=$((PASS + 1)); }
fail() { echo "  FAIL: $1 (expected: $2, got: $3)"; FAIL=$((FAIL + 1)); }
skip() { echo "  SKIP: $1 (file not found)"; SKIP=$((SKIP + 1)); }

# Extract a field from gap_measure output by label
gap_field() {
    local output="$1" label="$2"
    echo "$output" | grep "$label" | awk '{print $NF}'
}

# Extract ratio value (e.g., "4.25:1" -> "4.25")
gap_ratio() {
    local output="$1" label="$2"
    echo "$output" | grep "$label" | sed 's/.*ratio //' | sed 's/:1.*//'
}

# Check a value matches expected (string comparison)
check() {
    local desc="$1" expected="$2" actual="$3"
    if [ "$expected" = "$actual" ]; then
        pass "$desc"
    else
        fail "$desc" "$expected" "$actual"
    fi
}

# Check file size
check_size() {
    local desc="$1" expected="$2" file="$3"
    if [ ! -f "$file" ]; then
        skip "$desc"
        return
    fi
    local actual
    actual=$(wc -c < "$file" | tr -d ' ')
    check "$desc" "$expected" "$actual"
}

# Check width detection on first block
check_width() {
    local desc="$1" expected_width="$2" file="$3"
    if [ ! -f "$file" ]; then
        skip "$desc"
        return
    fi
    local output
    output=$("$BUILD/bpd_analyze" "$file" 2>&1)
    # Extract width from first data line (block 0)
    local actual
    actual=$(echo "$output" | awk 'NR>5 && NF>=3 {print $3; exit}')
    # "---" means no width detected = 0
    [ "$actual" = "---" ] && actual="0"
    check "$desc" "$expected_width" "$actual"
}

# Check false positive (no width detected on any of first 32 blocks)
check_false_positive() {
    local desc="$1" file="$2"
    if [ ! -f "$file" ]; then
        skip "$desc"
        return
    fi
    local output
    output=$("$BUILD/bpd_analyze" "$file" 2>&1)
    # Count blocks where width != "---"
    local detections
    detections=$(echo "$output" | awk 'NR>5 && NF>=3 && $3 != "---" {count++} END {print count+0}')
    if [ "$detections" = "0" ]; then
        pass "$desc (0 detections)"
    else
        fail "$desc" "0 detections" "$detections detections"
    fi
}

echo "============================================"
echo "Paper Number Verification"
echo "============================================"
echo ""

# --- File sizes (Section 6.2) ---
echo "--- File Sizes (Section 6.2) ---"
check_size "sensor_log size" "16777216" "$SENSOR_LOG"
check_size "intel_lab size" "53275272" "$INTEL_LAB"
check_size "mr size" "9970564" "$MR"
check_size "sao size" "7251944" "$SAO"
check_size "nasdaq size" "131839272" "$NASDAQ"

# --- Gap measurements (Sections 4.2, 7.2) ---
echo ""
echo "--- Gap Measurements (Sections 4.2, 7.2) ---"

if [ -f "$SENSOR_LOG" ]; then
    OUT=$("$BUILD/gap_measure" "$SENSOR_LOG" 2>&1)
    check "sensor_log R_ZSTD" "4.25" "$(gap_ratio "$OUT" "Plain ZSTD")"
    check "sensor_log R_shuffle" "173.54" "$(gap_ratio "$OUT" "Shuffle")"
    check "sensor_log G" "40.85x" "$(gap_field "$OUT" "Gap")"
else skip "sensor_log gap"; fi

if [ -f "$INTEL_LAB" ]; then
    OUT=$("$BUILD/gap_measure" "$INTEL_LAB" 2>&1)
    check "intel_lab R_ZSTD" "3.58" "$(gap_ratio "$OUT" "Plain ZSTD")"
    check "intel_lab R_shuffle" "4.26" "$(gap_ratio "$OUT" "Shuffle")"
    check "intel_lab G" "1.19x" "$(gap_field "$OUT" "Gap")"
else skip "intel_lab gap"; fi

if [ -f "$MR" ]; then
    OUT=$("$BUILD/gap_measure" "$MR" 2>&1)
    check "mr R_ZSTD" "2.72" "$(gap_ratio "$OUT" "Plain ZSTD")"
    check "mr R_shuffle" "2.76" "$(gap_ratio "$OUT" "Shuffle")"
    check "mr G" "1.02x" "$(gap_field "$OUT" "Gap")"
else skip "mr gap"; fi

if [ -f "$SAO" ]; then
    OUT=$("$BUILD/gap_measure" "$SAO" 2>&1)
    check "sao R_ZSTD" "1.25" "$(gap_ratio "$OUT" "Plain ZSTD")"
    check "sao R_shuffle" "1.26" "$(gap_ratio "$OUT" "Shuffle")"
    check "sao G" "1.01x" "$(gap_field "$OUT" "Gap")"
else skip "sao gap"; fi

if [ -f "$NASDAQ" ]; then
    OUT=$("$BUILD/gap_measure" "$NASDAQ" 2>&1)
    check "nasdaq R_ZSTD" "2.80" "$(gap_ratio "$OUT" "Plain ZSTD")"
    check "nasdaq R_shuffle" "2.80" "$(gap_ratio "$OUT" "Shuffle")"
    check "nasdaq G" "1.00x" "$(gap_field "$OUT" "Gap")"
else skip "nasdaq gap"; fi

# --- Width detection (Section 7.1) ---
echo ""
echo "--- Width Detection (Section 7.1) ---"
check_width "sensor_log width" "8" "$SENSOR_LOG"
check_width "intel_lab width" "24" "$INTEL_LAB"
check_width "sao width" "28" "$SAO"
check_width "nasdaq width (Alg 1)" "12" "$NASDAQ"

# mr: check that width=2 is detected on at least some blocks
if [ -f "$MR" ]; then
    MR_OUT=$("$BUILD/bpd_analyze" "$MR" 2>&1)
    MR_DETECTED=$(echo "$MR_OUT" | awk 'NR>5 && NF>=3 && $3 == "2" {count++} END {print count+0}')
    MR_TOTAL=$(echo "$MR_OUT" | awk 'NR>5 && NF>=3 {count++} END {print count+0}')
    if [ "$MR_DETECTED" -ge 8 ] && [ "$MR_DETECTED" -le 12 ]; then
        pass "mr width=2 on $MR_DETECTED/$MR_TOTAL blocks (~30%)"
    else
        fail "mr width=2 block count" "8-12 of 32" "$MR_DETECTED of $MR_TOTAL"
    fi
else skip "mr width detection"; fi

# --- Algorithm 2 validation (Section 7.1 byte-match table) ---
echo ""
echo "--- Algorithm 2 Validation (Section 7.1) ---"

if [ -f "$SENSOR_LOG" ]; then
    ACF_OUT=$("$BUILD/acf_analyze" "$SENSOR_LOG" 2>&1)
    ACF_W=$(echo "$ACF_OUT" | awk 'NR==6 {print $4}')
    check "sensor_log Alg2 width" "8" "$ACF_W"
else skip "sensor_log Alg2"; fi

if [ -f "$INTEL_LAB" ]; then
    ACF_OUT=$("$BUILD/acf_analyze" "$INTEL_LAB" 2>&1)
    ACF_W=$(echo "$ACF_OUT" | awk 'NR==6 {print $4}')
    check "intel_lab Alg2 width" "24" "$ACF_W"
else skip "intel_lab Alg2"; fi

if [ -f "$SAO" ]; then
    ACF_OUT=$("$BUILD/acf_analyze" "$SAO" 2>&1)
    ACF_W=$(echo "$ACF_OUT" | awk 'NR==6 {print $4}')
    check "sao Alg2 width" "28" "$ACF_W"
else skip "sao Alg2"; fi

if [ -f "$NASDAQ" ]; then
    ACF_OUT=$("$BUILD/acf_analyze" "$NASDAQ" 2>&1)
    ACF_W=$(echo "$ACF_OUT" | awk 'NR==6 {print $4}')
    check "nasdaq Alg2 width" "36" "$ACF_W"
else skip "nasdaq Alg2"; fi

# --- False positive check (Section 7.1) ---
echo ""
echo "--- False Positive Check (Section 7.1) ---"
for name in dickens mozilla nci ooffice osdb reymont samba webster xml; do
    check_false_positive "$name: no false positive" "$SILESIA/$name"
done

# --- Summary ---
echo ""
echo "============================================"
echo "Results: $PASS passed, $FAIL failed, $SKIP skipped"
echo "============================================"

if [ "$FAIL" -gt 0 ]; then
    echo "VERIFICATION FAILED — paper numbers do not match tool output"
    exit 1
else
    echo "ALL CHECKS PASSED"
    exit 0
fi
