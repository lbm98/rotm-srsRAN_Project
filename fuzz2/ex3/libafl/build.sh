#!/bin/bash
set -e

#
# Configuration
#

SCRIPT_DIR=`pwd`

RELEASE_DIR="$SCRIPT_DIR"/target/release
FUZZ_DIR="$SCRIPT_DIR"/fuzz

export LLVM_CONFIG=llvm-config-14

#
# Cleanup
#

rm -rf \
  crashes \
  fuzz \
  inputs \
  output \
  target

#
# Build the libafl compilers and libafl fuzzer
# Linking with libafl compiler automatically links the libafl fuzzer
#

cd "$SCRIPT_DIR"

cargo build --release

#
# Build libxml2 for fuzzing (no linking with libafl fuzzer)
#

#wget http://xmlsoft.org/download/libxml2-2.9.4.tar.gz
#tar xvf libxml2-2.9.4.tar.gz
#mkdir -p "$FUZZ_DIR"
#mv libxml2-2.9.4 "$FUZZ_DIR"/build

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

mkdir -p "$SCRIPT_DIR"/inputs
cp "$FUZZ_DIR"/build/test/*.xml "$SCRIPT_DIR"/inputs

#
# Build the binary for fuzzing (does linking with libafl fuzzer)
#

mkdir -p "$FUZZ_DIR"/bin

"$RELEASE_DIR"/libafl_cxx \
  "$SCRIPT_DIR"/target.cc \
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

taskset -c 4 ./fuzz/bin/fuzzer
taskset -c 6 ./fuzz/bin/fuzzer

#
# Rebuild after editing src/lib.rs
#

cd "$SCRIPT_DIR"

cargo build --release

"$RELEASE_DIR"/libafl_cxx \
  "$SCRIPT_DIR"/target.cc \
  -fuse-ld=lld \
  "$FUZZ_DIR"/build/install/lib/*.a \
  -I "$FUZZ_DIR"/build/install/include/libxml2 \
  -Wl,-Bstatic -lz -llzma -Wl,-Bdynamic \
  -o "$FUZZ_DIR"/bin/fuzzer