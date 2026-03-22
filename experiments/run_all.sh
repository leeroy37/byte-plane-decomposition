#!/bin/bash
# Master script: reproduces all paper tables.
# Run from the experiments/ directory after building and downloading data.
set -e

BUILDDIR="../build"

echo "=== Table 3.4: SAO Width Sweep ==="
$BUILDDIR/bpd_analyze ../data/silesia/sao --width 2
$BUILDDIR/bpd_analyze ../data/silesia/sao --width 4
$BUILDDIR/bpd_analyze ../data/silesia/sao --width 8
$BUILDDIR/bpd_analyze ../data/silesia/sao --width 14
$BUILDDIR/bpd_analyze ../data/silesia/sao --width 28
$BUILDDIR/bpd_analyze ../data/silesia/sao --width 32

echo ""
echo "=== Table 7.1: Width Detection ==="
for f in ../data/sensor_log.bin ../data/silesia/mr ../data/silesia/sao; do
    [ -f "$f" ] && $BUILDDIR/bpd_analyze "$f"
done
[ -f ../data/intel_lab.bin ] && $BUILDDIR/bpd_analyze ../data/intel_lab.bin

echo ""
echo "=== Table 7.1: False Positive Check (Silesia text files) ==="
for f in ../data/silesia/dickens ../data/silesia/webster ../data/silesia/reymont \
         ../data/silesia/samba ../data/silesia/xml ../data/silesia/nci \
         ../data/silesia/ooffice ../data/silesia/osdb ../data/silesia/mozilla; do
    [ -f "$f" ] && $BUILDDIR/bpd_analyze "$f"
done

echo ""
echo "=== Table 7.2: Universality Gap ==="
for f in ../data/sensor_log.bin ../data/silesia/mr ../data/silesia/sao; do
    [ -f "$f" ] && $BUILDDIR/gap_measure "$f"
done
[ -f ../data/intel_lab.bin ] && $BUILDDIR/gap_measure ../data/intel_lab.bin
[ -f ../data/nasdaq_itch_adds.bin ] && $BUILDDIR/gap_measure ../data/nasdaq_itch_adds.bin

echo ""
echo "=== Done ==="
