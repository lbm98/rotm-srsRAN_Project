#!/bin/bash
set -e

#
# Configuration
#

SCRIPT_DIR=`pwd`

CMAKE_SOURCE_DIR=$(readlink -f "$SCRIPT_DIR/../..")

RELEASE_DIR="$SCRIPT_DIR"/target/release
FUZZ_DIR="$SCRIPT_DIR"/srsran

export LLVM_CONFIG=llvm-config-14
export LIBAFL_EDGES_MAP_SIZE=1048576

#
# Build the libafl compilers and the fuzzer
# Linking with the libafl compiler automatically links the fuzzer
#

cd "$SCRIPT_DIR"

cargo build --release

#
# Build srsran (no linking with the libafl fuzzer)
#

mkdir -p "$FUZZ_DIR"/build
cd "$FUZZ_DIR"/build

cmake \
  -G Ninja \
  -DCMAKE_C_COMPILER="$RELEASE_DIR"/libafl_cc \
  -DCMAKE_CXX_COMPILER="$RELEASE_DIR"/libafl_cxx \
  -DCMAKE_LIBRARY_OUTPUT_DIRECTORY="$FUZZ_DIR"/build/install \
  -DCMAKE_ARCHIVE_OUTPUT_DIRECTORY="$FUZZ_DIR"/build/install \
  "$CMAKE_SOURCE_DIR"

cmake --build . --target rrc_ue_test_helpers srsran_rrc -v

mkdir "$FUZZ_DIR"/inputs
cp "$SCRIPT_DIR"/rrc_setup.bin "$FUZZ_DIR"/inputs

#
# Build the binary for fuzzing (does linking with the libafl fuzzer)
#

mkdir -p "$FUZZ_DIR"/bin

"$RELEASE_DIR"/libafl_cxx \
  "$SCRIPT_DIR"/srsran.cpp \
  -fuse-ld=lld \
  -I "$CMAKE_SOURCE_DIR" \
  -I "$CMAKE_SOURCE_DIR/include" \
  -I "$CMAKE_SOURCE_DIR/external" \
  -I "$CMAKE_SOURCE_DIR/external/fmt/include" \
  "$FUZZ_DIR"/build/install/*.a \
  -lgtest \
  -lmbedtls -lmbedx509 -lmbedcrypto \
  -o "$FUZZ_DIR"/bin/fuzzer

#
# Run the fuzzer
#

lsof -t -i:1337 | xargs -I {} sudo kill -9 {}

cd "$SCRIPT_DIR"

gnome-terminal

./srsran/bin/fuzzer --help

taskset -c 4 ./srsran/bin/fuzzer --input-dir srsran/inputs --output-dir srsran/output
taskset -c 6 ./srsran/bin/fuzzer --input-dir srsran/inputs --output-dir srsran/output