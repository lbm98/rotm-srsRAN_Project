#!/bin/bash
set -e

SCRIPT_DIR=$( cd -- "$( dirname -- "${BASH_SOURCE[0]}" )" &> /dev/null && pwd )

INPUT_DIR="$SCRIPT_DIR"/output/default/queue
OUTPUT_DIR="$SCRIPT_DIR"/output/default/queue_decoded

mkdir -p "$OUTPUT_DIR"

for file in "$INPUT_DIR"/*; do
    if [ -f "$file" ]; then
        filename=$(basename -- "$file")
        ./decode "$INPUT_DIR"/"$filename" "$OUTPUT_DIR"/"$filename"
    fi
done