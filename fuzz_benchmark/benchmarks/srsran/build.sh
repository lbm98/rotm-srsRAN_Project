#!/bin/bash
set -e

# Set these variables:
# - BUILD_OUT
# - LIB_OUT
# - CFLAGS
# - CXXFLAGS

mkdir "$BUILD_OUT"
cd "$BUILD_OUT"

cmake \
  -G Ninja \
  -DCMAKE_C_COMPILER=clang \
  -DCMAKE_CXX_COMPILER=clang++ \
  -DCMAKE_C_FLAGS="$CFLAGS" \
  -DCMAKE_CXX_FLAGS="$CXXFLAGS" \
  -DCMAKE_LIBRARY_OUTPUT_DIRECTORY="$LIB_OUT" \
  -DCMAKE_ARCHIVE_OUTPUT_DIRECTORY="$LIB_OUT" \
  "$CMAKE_SOURCE_DIR"

cmake --build . --target rrc_ue_test_helpers srsran_rrc -v