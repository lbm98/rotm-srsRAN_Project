#!/bin/bash
set -e

SCRIPT_DIR=`pwd`

AFL_DIR="$SCRIPT_DIR"/AFLplusplus
FUZZ_DIR="$SCRIPT_DIR"/fuzz
COVERAGE_DIR="$SCRIPT_DIR"/coverage

export LLVM_CONFIG=llvm-config-14

#
# Build AFL++
#

git clone \
  --branch v4.08c \
  --depth 1 \
  https://github.com/AFLplusplus/AFLplusplus \
  "$AFL_DIR"

cd "$AFL_DIR"

make source-only NO_NYX=1 NO_PYTHON=1

#
# Build libxml2 for fuzzing
#

git clone \
  --branch v2.12.0 \
  --depth 1 \
  https://github.com/GNOME/libxml2.git \
  "$FUZZ_DIR"/build

cd "$FUZZ_DIR"/build

CC="$AFL_DIR"/afl-clang-fast \
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

"$AFL_DIR"/afl-clang-fast++ \
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
CFLAGS="-fprofile-instr-generate -fcoverage-mapping" \
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
# Configure the fuzzer
#

sudo "$AFL_DIR"/afl-system-config

#
# Run the fuzzer
#

mkdir -p "$FUZZ_DIR"/info

AFL_FINAL_SYNC=1 \
"$AFL_DIR"/afl-fuzz \
  -i "$SCRIPT_DIR"/inputs \
  -o "$SCRIPT_DIR"/output \
  -s 42 \
  -M fuzzer01 \
  "$FUZZ_DIR"/bin/fuzzer \
  > "$FUZZ_DIR"/info/fuzz01.txt 2>&1 \
  &

#
# Get status of the fuzzer
#

"$AFL_DIR"/afl-whatsup "$SCRIPT_DIR"/output

#
# Plot statistics of the fuzzer
#

"$AFL_DIR"/afl-plot "$SCRIPT_DIR"/output/fuzzer01 $FUZZ_DIR/info/plot

#
# Stop fuzzer
#

pkill afl-fuzz

#
# Get coverage statistics
#

mkdir -p "$COVERAGE_DIR"/info

LLVM_PROFILE_FILE="$COVERAGE_DIR"/info/fuzzer.profraw \
"$COVERAGE_DIR"/bin/fuzzer \
  "$SCRIPT_DIR"/output/fuzzer01/queue/*

llvm-profdata merge \
  -sparse "$COVERAGE_DIR"/info/fuzzer.profraw \
  -o "$COVERAGE_DIR"/info/fuzzer.profdata

llvm-cov export \
  "$COVERAGE_DIR"/bin/fuzzer \
  -instr-profile="$COVERAGE_DIR"/info/fuzzer.profdata \
  -format=text -summary-only \
  > "$COVERAGE_DIR"/info/export.json