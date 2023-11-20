import os
import json
import pathlib
import subprocess

from invoke import task, Context

THIS_SCRIPT_DIR = pathlib.Path(__file__).parent.resolve()
CMAKE_SOURCE_DIR = (THIS_SCRIPT_DIR / '../..').resolve()

AFL_DIR_NAME = 'AFLplusplus'
AFL_DIR = THIS_SCRIPT_DIR / AFL_DIR_NAME
AFL_CLANG_FAST = AFL_DIR / 'afl-clang-fast'
AFL_CLANG_FASTPP = AFL_DIR / 'afl-clang-fast++'
AFL_FUZZ = AFL_DIR / 'afl-fuzz'

FUZZ_BUILD_DIR = THIS_SCRIPT_DIR / 'fuzz_build'
FUZZ_LIB_DIR = THIS_SCRIPT_DIR / 'fuzz_lib'
FUZZ_BIN_DIR = THIS_SCRIPT_DIR / 'fuzz_bin'
FUZZ_BIN = FUZZ_BIN_DIR / 'harness'
FUZZ_INFO_DIR = THIS_SCRIPT_DIR / 'fuzz_info'

COVERAGE_BUILD_DIR = THIS_SCRIPT_DIR / 'coverage_build'
COVERAGE_LIB_DIR = THIS_SCRIPT_DIR / 'coverage_lib'
COVERAGE_BIN_DIR = THIS_SCRIPT_DIR / 'coverage_bin'
COVERAGE_BIN = COVERAGE_BIN_DIR / 'harness'
COVERAGE_INFO_DIR = THIS_SCRIPT_DIR / 'coverage_info'

AFL_INPUT_DIR = THIS_SCRIPT_DIR / 'inputs'
AFL_OUTPUT_DIR = THIS_SCRIPT_DIR / 'output'

STANDALONE_FUZZ_TARGET_MAIN = THIS_SCRIPT_DIR / 'StandaloneFuzzTargetMain'
CORPUS_DIR = THIS_SCRIPT_DIR / 'output/default/queue'

os.environ['LLVM_CONFIG'] = 'llvm-config-14'


@task
def build_aflpp(ctx: Context):
    if AFL_DIR.exists():
        print('AFL++ already build...')
        return

    cmd = (
        f'git clone '
        f'--branch v4.08c '
        f'--depth 1 '
        f'https://github.com/AFLplusplus/AFLplusplus {AFL_DIR_NAME} '
    )

    ctx.run(cmd)
    ctx.run(f'cd {AFL_DIR_NAME} && make source-only NO_NYX=1 NO_PYTHON=1')


@task
def build_lib(ctx: Context):
    if FUZZ_BUILD_DIR.exists() or FUZZ_LIB_DIR.exists():
        print('srsRAN libraries already build...')
        return

    ctx.run(f'mkdir {FUZZ_BUILD_DIR}')

    with ctx.cd(FUZZ_BUILD_DIR):
        configure_cmd = (
            f'cmake '
            f'-G Ninja '
            f'-DCMAKE_C_COMPILER={AFL_CLANG_FAST} '
            f'-DCMAKE_CXX_COMPILER={AFL_CLANG_FASTPP} '
            f'-DCMAKE_LIBRARY_OUTPUT_DIRECTORY={FUZZ_LIB_DIR} '
            f'-DCMAKE_ARCHIVE_OUTPUT_DIRECTORY={FUZZ_LIB_DIR} '
            f'{CMAKE_SOURCE_DIR} '
        )

        ctx.run(configure_cmd)

        build_cmd = (
            f'cmake --build . --target rrc_ue_test_helpers srsran_rrc -v'
        )

        ctx.run(build_cmd)


@task
def build_coverage_lib(ctx: Context):
    if COVERAGE_BUILD_DIR.exists() or COVERAGE_LIB_DIR.exists():
        print('srsRAN coverage libraries already build...')
        return

    ctx.run(f'mkdir {COVERAGE_BUILD_DIR}')

    with ctx.cd(COVERAGE_BUILD_DIR):
        configure_cmd = (
            f'cmake '
            f'-G Ninja '
            f'-DCMAKE_C_COMPILER=clang '
            f'-DCMAKE_CXX_COMPILER=clang++ '
            f'-DCMAKE_C_FLAGS="-fprofile-instr-generate -fcoverage-mapping" '
            f'-DCMAKE_CXX_FLAGS="-fprofile-instr-generate -fcoverage-mapping" '
            f'-DCMAKE_LIBRARY_OUTPUT_DIRECTORY={COVERAGE_LIB_DIR} '
            f'-DCMAKE_ARCHIVE_OUTPUT_DIRECTORY={COVERAGE_LIB_DIR} '
            f'{CMAKE_SOURCE_DIR} '
        )

        ctx.run(configure_cmd)

        build_cmd = (
            f'cmake --build . --target rrc_ue_test_helpers srsran_rrc -v'
        )

        ctx.run(build_cmd)


@task
def build_harness(ctx: Context):
    if FUZZ_BIN_DIR.exists():
        print('harness already build...')
        return

    ctx.run(f'mkdir {FUZZ_BIN_DIR}')

    cmd = (
        f'{AFL_CLANG_FASTPP} '
        f'harness.cpp '
        f'-fsanitize=fuzzer '
        f'-fuse-ld=lld '
        f'-I {CMAKE_SOURCE_DIR} '
        f'-I {CMAKE_SOURCE_DIR}/include '
        f'-I {CMAKE_SOURCE_DIR}/external '
        f'-I {CMAKE_SOURCE_DIR}/external/fmt/include '
        f'{FUZZ_LIB_DIR}/*.a '
        f'-lgtest '
        f'-lmbedtls -lmbedx509 -lmbedcrypto '
        f'-o {FUZZ_BIN} '
    )

    ctx.run(cmd)


@task
def build_coverage_harness(ctx: Context):
    if COVERAGE_BIN_DIR.exists():
        print('coverage harness already build...')
        return

    ctx.run(f'mkdir {COVERAGE_BIN_DIR}')

    ctx.run(f'clang {STANDALONE_FUZZ_TARGET_MAIN}.c -c')

    cmd = (
        f'clang++ '
        f'{STANDALONE_FUZZ_TARGET_MAIN}.o '
        f'harness.cpp '
        f'-fprofile-instr-generate -fcoverage-mapping '
        f'-fuse-ld=lld '
        f'-I {CMAKE_SOURCE_DIR} '
        f'-I {CMAKE_SOURCE_DIR}/include '
        f'-I {CMAKE_SOURCE_DIR}/external '
        f'-I {CMAKE_SOURCE_DIR}/external/fmt/include '
        f'{COVERAGE_LIB_DIR}/*.a '
        f'-lgtest '
        f'-lmbedtls -lmbedx509 -lmbedcrypto '
        f'-o {COVERAGE_BIN} '
    )

    ctx.run(cmd)


@task
def generate_coverage_info(ctx: Context):
    if COVERAGE_INFO_DIR.exists():
        print('coverage info already build...')
        return

    ctx.run(f'mkdir {COVERAGE_INFO_DIR}')

    llvm_profraw = COVERAGE_INFO_DIR / 'fuzzer.profraw'
    llvm_profdata = COVERAGE_INFO_DIR / 'fuzzer.profdata'
    export_json = COVERAGE_INFO_DIR / 'export.json'
    totals_json = COVERAGE_INFO_DIR / 'totals.json'

    ctx.run(f'LLVM_PROFILE_FILE="{llvm_profraw}" {COVERAGE_BIN} {CORPUS_DIR}/*')
    ctx.run(f'llvm-profdata merge -sparse {llvm_profraw} -o {llvm_profdata}')
    ctx.run(f'llvm-cov export {COVERAGE_BIN} -instr-profile={llvm_profdata} -format=text -summary-only > {export_json}')

    with open(export_json, 'r') as export_file:
        data = json.load(export_file)
        totals = data['data'][0]['totals']

    with open(totals_json, 'w') as totals_file:
        json.dump(totals, totals_file, indent=4)


@task
def run_fuzzer(ctx: Context):
    ctx.run(f'mkdir -p {FUZZ_INFO_DIR}')

    # Default AFL_SYNC_TIME is 30 minutes

    cmd = (
        f'AFL_FINAL_SYNC=1 '  # still needs afl-cmin
        f'{AFL_FUZZ} '
        f'-i {AFL_INPUT_DIR} '
        f'-o {AFL_OUTPUT_DIR} '
        f'-s 42 '  # seed
        f'-V 10 '  # fuzz for x seconds
        f'-M fuzzer01 '  # main fuzzer 
        f'{FUZZ_BIN} '
        f'> {FUZZ_INFO_DIR}/fuzz01.txt 2>&1 '
        f'& '  # background process
    )

    ctx.run(cmd)

    num_cores = os.cpu_count()
    avl_cores = num_cores - 1

    for i in range(avl_cores):
        core_id = i + 2  # start counting from 2
        core_id_str = str(core_id).zfill(2)

        cmd = (
            f'{AFL_FUZZ} '
            f'-i {AFL_INPUT_DIR} '
            f'-o {AFL_OUTPUT_DIR} '
            f'-s 42 '  # seed
            f'-V 10 '  # fuzz for x seconds
            f'-S fuzzer{core_id_str} '  # secondary fuzzer 
            f'{FUZZ_BIN} '
            f'> {FUZZ_INFO_DIR}/fuzz{core_id_str}.txt 2>&1 '
            f'& '  # background process
        )

        ctx.run(cmd)


def main():
    pass


if __name__ == '__main__':
    main()
