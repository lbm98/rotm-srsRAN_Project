#!/bin/bash
set -e

SCRIPT_DIR=$( cd -- "$( dirname -- "${BASH_SOURCE[0]}" )" &> /dev/null && pwd )

DECODE_EXEC=$(readlink -f "$SCRIPT_DIR"/../utils/decode)

INPUT_DIR="$SCRIPT_DIR"/MY_CORPUS
OUTPUT_DIR="$SCRIPT_DIR"/MY_CORPUS_DECODED

rm -rf "$OUTPUT_DIR"
mkdir -p "$OUTPUT_DIR"

for file in "$INPUT_DIR"/*; do
    if [ -f "$file" ]; then
        filename=$(basename -- "$file")
        "$DECODE_EXEC" "$INPUT_DIR"/"$filename" "$OUTPUT_DIR"/"$filename"
    fi
done