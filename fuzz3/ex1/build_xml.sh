#!/bin/bash
set -e

#
# Configuration
#

SCRIPT_DIR=`pwd`

RELEASE_DIR="$SCRIPT_DIR"/target/release
FUZZ_DIR="$SCRIPT_DIR"/xml

export LLVM_CONFIG=llvm-config-14

#
# Build the libafl compilers and the fuzzer
# Linking with the libafl compiler automatically links the fuzzer
#

cd "$SCRIPT_DIR"

cargo build --release

#
# Build libxml2 (no linking with the libafl fuzzer)
#

git clone \
  --branch v2.9.4 \
  --depth 1 \
  https://github.com/GNOME/libxml2.git \
  "$FUZZ_DIR"/build

cd "$FUZZ_DIR"/build

CC="$RELEASE_DIR"/libafl_cc \
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

mkdir -p "$FUZZ_DIR"/inputs
cp "$FUZZ_DIR"/build/test/*.xml "$FUZZ_DIR"/inputs

#
# Build the binary for fuzzing (does linking with the libafl fuzzer)
#

mkdir -p "$FUZZ_DIR"/bin

"$RELEASE_DIR"/libafl_cxx \
  "$SCRIPT_DIR"/xml.cc \
  -fuse-ld=lld \
  "$FUZZ_DIR"/build/install/lib/*.a \
  -I "$FUZZ_DIR"/build/install/include/libxml2 \
  -Wl,-Bstatic -lz -llzma -Wl,-Bdynamic \
  -o "$FUZZ_DIR"/bin/fuzzer

#
# Run the fuzzer
#

lsof -t -i:1337 | xargs -I {} sudo kill -9 {}

cd "$SCRIPT_DIR"

gnome-terminal

./xml/bin/fuzzer --help

taskset -c 4 ./xml/bin/fuzzer --input-dir xml/inputs --output-dir xml/output
taskset -c 6 ./xml/bin/fuzzer --input-dir xml/inputs --output-dir xml/output