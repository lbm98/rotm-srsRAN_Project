```bash
sudo apt-get update
sudo apt-get install -y build-essential python3-dev automake cmake git flex bison libglib2.0-dev libpixman-1-dev python3-setuptools cargo libgtk-3-dev
```

```bash
PROJECT_DIR=`pwd`
```

### AFL

```bash
cd "$PROJECT_DIR"
git clone --depth 1 https://github.com/AFLplusplus/AFLplusplus
cd AFLplusplus
make source-only
```

```bash
cd "$PROJECT_DIR"
mkdir build
cd build
cmake -DCMAKE_C_COMPILER="$PROJECT_DIR"/AFLplusplus/afl-clang-fast -DCMAKE_CXX_COMPILER="$PROJECT_DIR"/AFLplusplus/afl-clang-fast++ ..
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
cmake --build . -j `nproc` --target rrc_ue_test -v #--clean-first
```