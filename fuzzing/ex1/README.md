```bash
mkdir inputs
mkdir output

xxd -i inputs/input.bin

xxd -i crash-*
xxd -i oom-*

-DCMAKE_C_COMPILER=clang -DCMAKE_CXX_COMPILER=clang++

./fuzzing_target output inputs
./fuzzing_target -rss_limit_mb=8192 output inputs

./StandaloneFuzzTargetMain inputs/*
./StandaloneFuzzTargetMain crash-*

mkdir decoded
./decode inputs/input.bin decoded/input.bin
```