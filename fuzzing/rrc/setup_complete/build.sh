#!/bin/bash
set -e

CMAKE_BUILD_DIR=$(realpath ../../../cmake-build-debug)

rm -rf bin inputs output inputs_decoded output_decoded
mkdir bin inputs output inputs_decoded output_decoded

cmake --build "$CMAKE_BUILD_DIR" --target fuzzing_rrc_setup_complete_fuzz_target -v
cmake --build "$CMAKE_BUILD_DIR" --target fuzzing_rrc_setup_complete_run_target
cmake --build "$CMAKE_BUILD_DIR" --target fuzzing_rrc_setup_complete_generate_input
cmake --build "$CMAKE_BUILD_DIR" --target fuzzing_rrc_setup_complete_decode

./bin/generate_input
./bin/decode inputs/input.bin inputs_decoded/input.bin
./bin/run_target inputs/input.bin
./bin/fuzz_target output inputs

./bin/run_target crash-*

for file in output/*; do
    ./bin/decode "$file" output_decoded/$(basename -- "$file")
done
