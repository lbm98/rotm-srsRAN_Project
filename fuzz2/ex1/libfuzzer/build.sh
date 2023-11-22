#!/bin/bash
set -e

SCRIPT_DIR=`pwd`

FUZZ_DIR="$SCRIPT_DIR"/fuzz
COVERAGE_DIR="$SCRIPT_DIR"/coverage

export LLVM_CONFIG=llvm-config-14

#
# Build libxml2 for fuzzing
#

git clone \
  --branch v2.12.0 \
  --depth 1 \
  https://github.com/GNOME/libxml2.git \
  "$FUZZ_DIR"/build

cd "$FUZZ_DIR"/build

CC=clang \
CFLAGS="-fsanitize=fuzzer-no-link" \
./autogen.sh \
  --disable-shared \
  --without-debug \
  --without-ftp \
  --without-http \
  --without-legacy \
  --without-python \
  --prefix "$FUZZ_DIR"/build/install

make -j$(nproc)
make install

mkdir -p "$SCRIPT_DIR"/inputs
cp "$FUZZ_DIR"/build/test/*.xml "$SCRIPT_DIR"/inputs

#
# Build the binary for fuzzing
#

mkdir -p "$FUZZ_DIR"/bin

clang++ \
  "$SCRIPT_DIR"/target.cc \
  -fsanitize=fuzzer \
  -fuse-ld=lld \
  "$FUZZ_DIR"/build/install/lib/*.a \
  -I "$FUZZ_DIR"/build/install/include/libxml2 \
  -Wl,-Bstatic -lz -llzma -Wl,-Bdynamic \
  -o "$FUZZ_DIR"/bin/fuzzer

#
# Build libxml2 for coverage
#

git clone \
  --branch v2.12.0 \
  --depth 1 \
  https://gitlab.gnome.org/GNOME/libxml2.git/ \
  "$COVERAGE_DIR"/build

cd "$COVERAGE_DIR"/build

CC=clang \
CFLAGS="-fprofile-instr-generate -fcoverage-mapping"
./autogen.sh \
  --disable-shared \
  --without-debug \
  --without-ftp \
  --without-http \
  --without-legacy \
  --without-python \
  --prefix "$COVERAGE_DIR"/build/install

make -j$(nproc)
make install

#
# Build the binary for coverage
#

mkdir -p "$COVERAGE_DIR"/bin

clang "$SCRIPT_DIR"/StandaloneFuzzTargetMain.c -c -o "$COVERAGE_DIR"/StandaloneFuzzTargetMain.o

clang++ \
  "$SCRIPT_DIR"/target.cc \
  "$COVERAGE_DIR"/StandaloneFuzzTargetMain.o \
  -fuse-ld=lld \
  -fprofile-instr-generate -fcoverage-mapping \
  "$COVERAGE_DIR"/build/install/lib/*.a \
  -I "$COVERAGE_DIR"/build/install/include/libxml2 \
  -Wl,-Bstatic -lz -llzma -Wl,-Bdynamic \
  -o "$COVERAGE_DIR"/bin/fuzzer

#
# Run the fuzzer
#

# Extracts input seeds from all directories, but only outputs new testcases to the first directory

rm -rf "$SCRIPT_DIR"/output
mkdir -p "$SCRIPT_DIR"/output

mkdir -p $FUZZ_DIR/info

"$FUZZ_DIR"/bin/fuzzer \
  -seed=42 \
  "$SCRIPT_DIR"/output \
  "$SCRIPT_DIR"/inputs \
  -print_final_stats=1 \
  > "$FUZZ_DIR"/info/fuzz01.txt 2>&1
