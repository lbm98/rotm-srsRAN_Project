import os
import json
import pathlib

from invoke import task, Context
from dataclasses import dataclass

THIS_SCRIPT_DIR = pathlib.Path(__file__).parent.resolve()
CMAKE_SOURCE_DIR = THIS_SCRIPT_DIR.parent.parent.resolve()


@dataclass
class AFL:
    BUILD_DIR = THIS_SCRIPT_DIR / 'AFLplusplus'
    CLANG_FAST = BUILD_DIR / 'afl-clang-fast'
    CLANG_FASTPP = BUILD_DIR / 'afl-clang-fast++'
    FUZZ = BUILD_DIR / 'afl-fuzz'
    INPUT_DIR = THIS_SCRIPT_DIR / 'inputs'
    OUTPUT_DIR = THIS_SCRIPT_DIR / 'output'
    CORPUS_DIR = OUTPUT_DIR / 'fuzzer01/queue'


@dataclass
class FUZZ:
    BUILD_DIR = THIS_SCRIPT_DIR / 'fuzz_build'
    LIB_DIR = THIS_SCRIPT_DIR / 'fuzz_lib'
    BIN_DIR = THIS_SCRIPT_DIR / 'fuzz_bin'
    BIN = BIN_DIR / 'harness'
    INFO_DIR = THIS_SCRIPT_DIR / 'fuzz_info'


@dataclass
class COVERAGE:
    BUILD_DIR = THIS_SCRIPT_DIR / 'coverage_build'
    LIB_DIR = THIS_SCRIPT_DIR / 'coverage_lib'
    BIN_DIR = THIS_SCRIPT_DIR / 'coverage_bin'
    BIN = BIN_DIR / 'harness'
    INFO_DIR = THIS_SCRIPT_DIR / 'coverage_info'

    LLVM_PROFRAW = INFO_DIR / 'fuzzer.profraw'
    LLVM_PROFDATA = INFO_DIR / 'fuzzer.profdata'
    EXPORT_JSON = INFO_DIR / 'export.json'
    TOTALS_JSON = INFO_DIR / 'totals.json'


STANDALONE_FUZZ_TARGET_MAIN = THIS_SCRIPT_DIR / 'StandaloneFuzzTargetMain'
CORPUS_DIR = THIS_SCRIPT_DIR / 'output/default/queue'

os.environ['LLVM_CONFIG'] = 'llvm-config-14'


@task
def build_aflpp(ctx: Context):
    if AFL.BUILD_DIR.exists():
        print('AFL++ already build...')
        return

    ctx.run(
        f'git clone '
        f'--branch v4.08c '
        f'--depth 1 '
        f'https://github.com/AFLplusplus/AFLplusplus {AFL.BUILD_DIR} '
    )

    ctx.run(f'cd {AFL.BUILD_DIR} && make source-only NO_NYX=1 NO_PYTHON=1')


@task
def build_lib(ctx: Context):
    if FUZZ.BUILD_DIR.exists() or FUZZ.LIB_DIR.exists():
        print('srsRAN libraries already build...')
        return

    ctx.run(f'mkdir {FUZZ.BUILD_DIR}')

    with ctx.cd(FUZZ.BUILD_DIR):
        ctx.run(
            f'cmake '
            f'-G Ninja '
            f'-DCMAKE_C_COMPILER={AFL.CLANG_FAST} '
            f'-DCMAKE_CXX_COMPILER={AFL.CLANG_FASTPP} '
            f'-DCMAKE_LIBRARY_OUTPUT_DIRECTORY={FUZZ.LIB_DIR} '
            f'-DCMAKE_ARCHIVE_OUTPUT_DIRECTORY={FUZZ.LIB_DIR} '
            f'{CMAKE_SOURCE_DIR} '
        )

        ctx.run('cmake --build . --target rrc_ue_test_helpers srsran_rrc -v')


@task
def build_coverage_lib(ctx: Context):
    if COVERAGE.BUILD_DIR.exists() or COVERAGE.LIB_DIR.exists():
        print('srsRAN coverage libraries already build...')
        return

    ctx.run(f'mkdir {COVERAGE.BUILD_DIR}')

    with ctx.cd(COVERAGE.BUILD_DIR):
        ctx.run(
            f'cmake '
            f'-G Ninja '
            f'-DCMAKE_C_COMPILER=clang '
            f'-DCMAKE_CXX_COMPILER=clang++ '
            f'-DCMAKE_C_FLAGS="-fprofile-instr-generate -fcoverage-mapping" '
            f'-DCMAKE_CXX_FLAGS="-fprofile-instr-generate -fcoverage-mapping" '
            f'-DCMAKE_LIBRARY_OUTPUT_DIRECTORY={COVERAGE.LIB_DIR} '
            f'-DCMAKE_ARCHIVE_OUTPUT_DIRECTORY={COVERAGE.LIB_DIR} '
            f'{CMAKE_SOURCE_DIR} '
        )

        ctx.run('cmake --build . --target rrc_ue_test_helpers srsran_rrc -v')


@task
def build_harness(ctx: Context):
    if FUZZ.BIN_DIR.exists():
        print('harness already build...')
        return

    ctx.run(f'mkdir {FUZZ.BIN_DIR}')

    ctx.run(
        f'{AFL.CLANG_FASTPP} '
        f'harness.cpp '
        f'-fuse-ld=lld '
        f'-I {CMAKE_SOURCE_DIR} '
        f'-I {CMAKE_SOURCE_DIR}/include '
        f'-I {CMAKE_SOURCE_DIR}/external '
        f'-I {CMAKE_SOURCE_DIR}/external/fmt/include '
        f'{FUZZ.LIB_DIR}/*.a '
        f'-lgtest '
        f'-lmbedtls -lmbedx509 -lmbedcrypto '
        f'-o {FUZZ.BIN} '
    )


@task
def build_coverage_harness(ctx: Context):
    if COVERAGE.BIN_DIR.exists():
        print('coverage harness already build...')
        return

    ctx.run(f'mkdir {COVERAGE.BIN_DIR}')

    ctx.run(f'clang {STANDALONE_FUZZ_TARGET_MAIN}.c -c')

    ctx.run(
        f'clang++ '
        f'{STANDALONE_FUZZ_TARGET_MAIN}.o '
        f'harness.cpp '
        f'-fprofile-instr-generate -fcoverage-mapping '
        f'-fuse-ld=lld '
        f'-I {CMAKE_SOURCE_DIR} '
        f'-I {CMAKE_SOURCE_DIR}/include '
        f'-I {CMAKE_SOURCE_DIR}/external '
        f'-I {CMAKE_SOURCE_DIR}/external/fmt/include '
        f'{COVERAGE.LIB_DIR}/*.a '
        f'-lgtest '
        f'-lmbedtls -lmbedx509 -lmbedcrypto '
        f'-o {COVERAGE.BIN} '
    )


@task
def generate_coverage_info(ctx: Context):
    if COVERAGE.INFO_DIR.exists():
        print('coverage info already build...')
        return

    ctx.run(f'mkdir {COVERAGE.INFO_DIR}')

    ctx.run(
        f'LLVM_PROFILE_FILE="{COVERAGE.LLVM_PROFRAW}" '
        f'{COVERAGE.BIN} '
        f'{CORPUS_DIR}/* '
    )

    ctx.run(
        f'llvm-profdata merge '
        f'-sparse {COVERAGE.LLVM_PROFRAW} '
        f'-o {COVERAGE.LLVM_PROFDATA} '
    )

    ctx.run(
        f'llvm-cov export {COVERAGE.BIN} '
        f'-instr-profile={COVERAGE.LLVM_PROFDATA} '
        f'-format=text -summary-only '
        f'> {COVERAGE.EXPORT_JSON}'
    )

    with open(COVERAGE.EXPORT_JSON, 'r') as export_file:
        data = json.load(export_file)
        totals = data['data'][0]['totals']

    with open(COVERAGE.TOTALS_JSON, 'w') as totals_file:
        json.dump(totals, totals_file, indent=4)


@task
def run_fuzzer(ctx: Context):
    ctx.run(f'mkdir -p {FUZZ.INFO_DIR}')

    # Default AFL_SYNC_TIME is 30 minutes

    ctx.run(
        f'AFL_FINAL_SYNC=1 '  # still needs afl-cmin
        f'{AFL.FUZZ} '
        f'-i {AFL.INPUT_DIR} '
        f'-o {AFL.OUTPUT_DIR} '
        f'-s 42 '  # seed
        f'-V 10 '  # fuzz for x seconds
        f'-M fuzzer01 '  # main fuzzer 
        f'{FUZZ.BIN} '
        f'> {FUZZ.INFO_DIR}/fuzz01.txt 2>&1 '
        f'& '  # background process
    )

    num_cores = os.cpu_count()
    free_cores = num_cores - 1

    for i in range(free_cores):
        core_id = i + 2  # start counting from 2
        core_id_str = str(core_id).zfill(2)

        ctx.run(
            f'{AFL.FUZZ} '
            f'-i {AFL.INPUT_DIR} '
            f'-o {AFL.OUTPUT_DIR} '
            f'-s 42 '  # seed
            f'-V 10 '  # fuzz for x seconds
            f'-S fuzzer{core_id_str} '  # secondary fuzzer 
            f'{FUZZ.BIN} '
            f'> {FUZZ.INFO_DIR}/fuzz{core_id_str}.txt 2>&1 '
            f'& '  # background process
        )


def main():
    ctx = Context()
    run_fuzzer(ctx)


if __name__ == '__main__':
    main()
