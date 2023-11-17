## AFL++

- https://github.com/antonio-morales/Fuzzing101
- https://github.com/fuzzstati0n/fuzzgoat
- https://github.com/vanhauser-thc/afl-cov

## LTO + Persistent mode

- https://github.com/AFLplusplus/AFLplusplus/blob/stable/instrumentation/README.lto.md
- https://github.com/AFLplusplus/AFLplusplus/blob/stable/instrumentation/README.persistent_mode.md

TODO:

- Checkout GTest and build your own static library to link against

### fuzz_setup_request_non_persistent

```bash
sudo apt-get install -y lld

PROJECT_DIR=`pwd`
FUZZ_DIR="$PROJECT_DIR"/fuzz

export LLVM_CONFIG="llvm-config-14"

cd "$FUZZ_DIR"
git clone --depth 1 https://github.com/AFLplusplus/AFLplusplus
cd AFLplusplus
make source-only NO_NYX=1 NO_PYTHON=1

cd "$PROJECT_DIR"
mkdir built-with-lto
cd built-with-lto

cmake \
-DCMAKE_C_COMPILER="$FUZZ_DIR"/AFLplusplus/afl-clang-lto \
-DCMAKE_CXX_COMPILER="$FUZZ_DIR"/AFLplusplus/afl-clang-lto++ \
..

cmake --build . --target fuzz_setup_request_non_persistent -v

cd "$PROJECT_DIR"
sudo ./AFLplusplus/afl-system-config
"$FUZZ_DIR"/AFLplusplus/afl-fuzz -i "$FUZZ_DIR"/input -o "$FUZZ_DIR"/output -s 123 -- "$PROJECT_DIR"/built-with-lto/fuzz/fuzz_setup_request_non_persistent @@
```

### fuzz_setup_request_persistent

```bash
cd "$PROJECT_DIR"
cd built-with-lto
cmake --build . --target fuzz_setup_request_persistent -v

cd "$PROJECT_DIR"
sudo ./AFLplusplus/afl-system-config
"$FUZZ_DIR"/AFLplusplus/afl-fuzz -i "$FUZZ_DIR"/input -o "$FUZZ_DIR"/output -s 123 -- "$PROJECT_DIR"/built-with-lto/fuzz/fuzz_setup_request_persistent
```