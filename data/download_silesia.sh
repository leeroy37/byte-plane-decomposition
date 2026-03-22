#!/bin/bash
# Download and extract the Silesia Compression Corpus.
set -e
mkdir -p data/silesia
cd data/silesia
echo "Downloading Silesia Corpus (~67 MB)..."
curl -L -o silesia.zip "https://sun.aei.polsl.pl/~sdeor/corpus/silesia.zip"
unzip -o silesia.zip
rm silesia.zip
echo "Done. Files in data/silesia/"
ls -lh
