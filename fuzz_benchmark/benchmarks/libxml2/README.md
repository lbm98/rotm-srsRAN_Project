```bash
CXX="clang++" \
CXXFLAGS="-fprofile-instr-generate -fcoverage-mapping" \
OUT="/home/lbm/rotm4/rotm-srsRAN_Project/fuzz_benchmark/benchmarks/libxml2/out" \
../build.sh
```