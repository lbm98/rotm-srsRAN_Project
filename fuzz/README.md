```bash
sudo apt-get update
sudo apt-get install -y build-essential python3-dev automake cmake git flex bison libglib2.0-dev libpixman-1-dev python3-setuptools cargo libgtk-3-dev
```

```bash
PROJECT_DIR=`pwd`
```

### AFL++

- https://github.com/antonio-morales/Fuzzing101
- https://github.com/fuzzstati0n/fuzzgoat
- https://github.com/vanhauser-thc/afl-cov

TODO:

- Add pcap dict

```bash
PROJECT_DIR=`pwd`

export LLVM_CONFIG=llvm-config-14
export AFL_USE_ASAN=1
export AFL_CC_COMPILER=LLVM

cd "$PROJECT_DIR"
git clone --depth 1 https://github.com/AFLplusplus/AFLplusplus
cd AFLplusplus
make source-only NO_NYX=1 NO_PYTHON=1

cd "$PROJECT_DIR"
mkdir build
cd build

cmake \
-DCMAKE_C_COMPILER="$PROJECT_DIR"/AFLplusplus/afl-cc \
-DCMAKE_CXX_COMPILER="$PROJECT_DIR"/AFLplusplus/afl-c++ \
..

cmake --build . --target rrc_ue_setup_proc_fuzz_test
cmake --build . --target rrc_ue_setup_proc_fuzz_test -v --clean-first

cd "$PROJECT_DIR"
sudo ./AFLplusplus/afl-system-config
./AFLplusplus/afl-fuzz -i "$PROJECT_DIR"/fuzz/input -o "$PROJECT_DIR"/fuzz/output -s 123 -- "$PROJECT_DIR"/build/tests/unittests/rrc/rrc_ue_setup_proc_fuzz_test @@

time "$PROJECT_DIR"/build/tests/unittests/rrc/rrc_ue_setup_proc_fuzz_test "$PROJECT_DIR"/fuzz/input/rrc_setup.bin
```

### fuzztest

```bash
cd "$PROJECT_DIR"
mkdir build
cd build
cmake \
-DCMAKE_C_COMPILER=clang \
-DCMAKE_CXX_COMPILER=clang++ \
-DCMAKE_BUILD_TYPE=RelWithDebug \
-DFUZZTEST_FUZZING_MODE=on \
..
cmake --build . -j `nproc` --target rrc_ue_test -v
```