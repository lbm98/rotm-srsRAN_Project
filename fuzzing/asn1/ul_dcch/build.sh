#!/bin/bash
set -e

CMAKE_BUILD_DIR=$(realpath ../../../cmake-build-debug)
CMAKE_TARGET_DIR="$CMAKE_BUILD_DIR"/fuzzing/asn1/ul_dcch

rm -rf bin inputs output inputs_decoded output_decoded
mkdir bin inputs output inputs_decoded output_decoded

cmake --build "$CMAKE_BUILD_DIR" --target fuzzing_asn1_ul_dcch_fuzz_target
cmake --build "$CMAKE_BUILD_DIR" --target fuzzing_asn1_ul_dcch_run_target
cmake --build "$CMAKE_BUILD_DIR" --target fuzzing_asn1_ul_dcch_generate_input
cmake --build "$CMAKE_BUILD_DIR" --target fuzzing_asn1_ul_dcch_decode

"$CMAKE_TARGET_DIR"/generate_input
"$CMAKE_TARGET_DIR"/decode inputs/input.bin inputs_decoded/input.bin
"$CMAKE_TARGET_DIR"/run_target inputs/input.bin
"$CMAKE_TARGET_DIR"/fuzz_target output inputs

"$CMAKE_TARGET_DIR"/run_target crash-*

for file in output/*; do
    ./bin/decode "$file" output_decoded/$(basename -- "$file")
done
