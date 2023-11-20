## Fuzzing 101

- https://github.com/antonio-morales/Fuzzing101
- https://github.com/epi052/fuzzing-101-solutions

## Technical whitepapers

- https://aflplus.plus/docs/technical_details/
- https://lcamtuf.coredump.cx/afl/technical_details.txt

## Parallelization

- https://aflplus.plus/docs/parallel_fuzzing/

# Tasks library

- https://github.com/epi052/fuzzing-101-solutions/blob/main/exercise-5/tasks.py

## General fuzzing guidelines

- https://llvm.org/docs/LibFuzzer.html

```
The fuzzing engine will execute the fuzz target many times with different inputs in the same process.
It must tolerate any kind of input (empty, huge, malformed, etc).
It must not exit() on any input.
It may use threads but ideally all threads should be joined at the end of the function.
It must be as deterministic as possible. Non-determinism (e.g. random decisions not based on the input bytes) will make fuzzing inefficient.
It must be fast. Try avoiding cubic or greater complexity, logging, or excessive memory consumption.
Ideally, it should not modify any global state (although that’s not strict).
Usually, the narrower the target the better. E.g. if your target can parse several data formats, split it into several targets, one per format.
```

- https://aflplus.plus/docs/fuzzing_in_depth/

```
Avoid instrumenting shared libraries that could get put in /usr/lib
```

```
The afl-gotcpu utility can help you understand if you still have idle CPU capacity on your system.
(It won’t tell you about memory bandwidth, cache misses, or similar factors, but they are less likely to be a concern.)
```

## Dependencies

```bash
sudo apt-get install -y lld

python3 -m venv venv
venv/bin/pip install -r requirements.txt
```