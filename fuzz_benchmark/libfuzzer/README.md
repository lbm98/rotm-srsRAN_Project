- https://github.com/llvm/llvm-project/tree/main/compiler-rt/lib/fuzzer
- https://llvm.org/docs/LibFuzzer.html
- https://github.com/google/fuzzing/blob/master/tutorial/libFuzzerTutorial.md
- https://github.com/google/fuzzing/blob/master/docs/structure-aware-fuzzing.md

```bash
wget https://raw.githubusercontent.com/llvm/llvm-project/main/compiler-rt/lib/fuzzer/standalone/StandaloneFuzzTargetMain.c
```

```bash
PROJECT_DIR=$(pwd)
CMAKE_SOURCE_DIR=$(readlink -f "$PROJECT_DIR/../..")

mkdir build
cd build

cmake \
-G Ninja \
-DCMAKE_C_COMPILER=clang \
-DCMAKE_CXX_COMPILER=clang++ \
-DCMAKE_C_FLAGS="-fsanitize=fuzzer-no-link" \
-DCMAKE_CXX_FLAGS="-fsanitize=fuzzer-no-link" \
-DCMAKE_LIBRARY_OUTPUT_DIRECTORY="$PROJECT_DIR/lib" \
-DCMAKE_ARCHIVE_OUTPUT_DIRECTORY="$PROJECT_DIR/lib" \
"$CMAKE_SOURCE_DIR"

cmake --build . --target rrc_ue_test_helpers srsran_rrc -v

cd "$PROJECT_DIR"

clang++ \
target.cpp \
-fsanitize=fuzzer \
-fuse-ld=lld \
-I "$CMAKE_SOURCE_DIR" \
-I "$CMAKE_SOURCE_DIR/include" \
-I "$CMAKE_SOURCE_DIR/external" \
-I "$CMAKE_SOURCE_DIR/external/fmt/include" \
lib/*.a \
-lgtest \
-lmbedtls -lmbedx509 -lmbedcrypto \
-o fuzzer

./clean.sh

# Read inputs from all directories, but only write to the first directory
./fuzzer -seed=42 -jobs=$(nproc) -workers=$(nproc) MY_CORPUS/ seeds/

mkdir MINIMIZED_CORPUS
./fuzzer -merge=1 MINIMIZED_CORPUS MY_CORPUS
```

```bash
PROJECT_DIR=$(pwd)
CMAKE_SOURCE_DIR=$(readlink -f "$PROJECT_DIR/../..")

mkdir build_coverage
cd build_coverage

cmake \
-G Ninja \
-DCMAKE_C_COMPILER=clang \
-DCMAKE_CXX_COMPILER=clang++ \
-DCMAKE_C_FLAGS="-fprofile-instr-generate -fcoverage-mapping" \
-DCMAKE_CXX_FLAGS="-fprofile-instr-generate -fcoverage-mapping" \
-DCMAKE_LIBRARY_OUTPUT_DIRECTORY="$PROJECT_DIR/lib_coverage" \
-DCMAKE_ARCHIVE_OUTPUT_DIRECTORY="$PROJECT_DIR/lib_coverage" \
"$CMAKE_SOURCE_DIR"

cmake --build . --target rrc_ue_test_helpers srsran_rrc -v

cd "$PROJECT_DIR"

clang StandaloneFuzzTargetMain.c -c

clang++ \
StandaloneFuzzTargetMain.o \
target.cpp \
-fprofile-instr-generate -fcoverage-mapping \
-fuse-ld=lld \
-I "$CMAKE_SOURCE_DIR" \
-I "$CMAKE_SOURCE_DIR/include" \
-I "$CMAKE_SOURCE_DIR/external" \
-I "$CMAKE_SOURCE_DIR/external/fmt/include" \
lib_coverage/*.a \
-lgtest \
-lmbedtls -lmbedx509 -lmbedcrypto \
-o fuzzer_coverage

LLVM_PROFILE_FILE="fuzzer.profraw" ./fuzzer_coverage MY_CORPUS/*

# Can combine multiple raw profiles
llvm-profdata merge -sparse fuzzer.profraw -o fuzzer.profdata

llvm-cov report ./fuzzer_coverage -instr-profile=fuzzer.profdata
llvm-cov export ./fuzzer_coverage -instr-profile=fuzzer.profdata -format=text -summary-only > h.json

```
