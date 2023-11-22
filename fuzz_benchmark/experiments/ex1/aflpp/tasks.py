import os
import sys
import json
import pathlib

from invoke import Context, task
from dataclasses import dataclass

THIS_SCRIPT_DIR = pathlib.Path(__file__).parent.resolve()


def find_requirements_file(start_dir):
    current_dir = start_dir

    while current_dir != current_dir.parent:
        requirements_file = current_dir / 'requirements.txt'
        if requirements_file.is_file():
            return str(current_dir)
        current_dir = current_dir.parent

    assert False


sys.path.append(find_requirements_file(THIS_SCRIPT_DIR))

import targets.libxml2 as target

os.environ['LLVM_CONFIG'] = 'llvm-config-14'


@dataclass
class AFL:
    BUILD_DIR = THIS_SCRIPT_DIR / 'AFLplusplus'
    CLANG_FAST = BUILD_DIR / 'afl-clang-fast'
    CLANG_FASTPP = BUILD_DIR / 'afl-clang-fast++'
    FUZZ = BUILD_DIR / 'afl-fuzz'
    SEED_DIR = THIS_SCRIPT_DIR / 'inputs'
    OUTPUT_DIR = THIS_SCRIPT_DIR / 'output'
    CORPUS_DIR = OUTPUT_DIR / 'fuzzer01/queue'
    SYSTEM_CONFIG = BUILD_DIR / 'afl-system-config'


@dataclass
class FUZZ:
    DIR = THIS_SCRIPT_DIR / 'fuzz'
    SOURCE_DIR = DIR / 'source'
    BUILD_DIR = DIR / 'build'
    INSTALL_DIR = BUILD_DIR / 'install'
    LIB_DIR = DIR / 'lib'
    BIN_DIR = DIR / 'bin'
    BIN = BIN_DIR / 'fuzzer'
    INFO_DIR = DIR / 'info'


@dataclass
class COVERAGE:
    DIR = THIS_SCRIPT_DIR / 'coverage'
    BUILD_DIR = DIR / 'build'
    INSTALL_DIR = BUILD_DIR / 'install'
    LIB_DIR = DIR / 'lib'
    BIN_DIR = DIR / 'bin'
    BIN = BIN_DIR / 'fuzzer'
    INFO_DIR = DIR / 'info'

    LLVM_PROFRAW = INFO_DIR / 'fuzzer.profraw'
    LLVM_PROFDATA = INFO_DIR / 'fuzzer.profdata'
    EXPORT_JSON = INFO_DIR / 'export.json'
    TOTALS_JSON = INFO_DIR / 'totals.json'


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
    target.build_lib(
        ctx=ctx,
        cc=AFL.CLANG_FAST,
        cxx=AFL.CLANG_FASTPP,
        cflags='',
        cxxflags='',
        build_dir=FUZZ.BUILD_DIR,
        install_dir=FUZZ.INSTALL_DIR,
        lib_dir=FUZZ.LIB_DIR,
        seed_dir=AFL.SEED_DIR
    )


@task
def build_bin(ctx: Context):
    target.build_bin(
        ctx=ctx,
        cc=AFL.CLANG_FAST,
        cxx=AFL.CLANG_FASTPP,
        cflags='-fsanitize=fuzzer',
        cxxflags='-fsanitize=fuzzer',
        install_dir=FUZZ.INSTALL_DIR,
        lib_dir=FUZZ.LIB_DIR,
        bin_dir=FUZZ.BIN_DIR,
        bin=FUZZ.BIN,
        insert_main=False
    )


@task
def build_coverage_lib(ctx: Context):
    target.build_lib(
        ctx=ctx,
        cc='clang',
        cxx='clang++',
        cflags='-fprofile-instr-generate -fcoverage-mapping',
        cxxflags='-fprofile-instr-generate -fcoverage-mapping',
        build_dir=COVERAGE.BUILD_DIR,
        install_dir=COVERAGE.INSTALL_DIR,
        lib_dir=COVERAGE.LIB_DIR,
        seed_dir=None
    )


@task
def build_coverage_bin(ctx: Context):
    target.build_bin(
        ctx=ctx,
        cc='clang',
        cxx='clang++',
        cflags='-fprofile-instr-generate -fcoverage-mapping',
        cxxflags='-fprofile-instr-generate -fcoverage-mapping',
        install_dir=COVERAGE.INSTALL_DIR,
        lib_dir=COVERAGE.LIB_DIR,
        bin_dir=COVERAGE.BIN_DIR,
        bin=COVERAGE.BIN,
        insert_main=True
    )


@task
def setup_fuzzer(ctx: Context):
    ctx.run(f'{AFL.SYSTEM_CONFIG}')


@task
def run_fuzzer(ctx: Context):
    ctx.run(f'mkdir -p {FUZZ.INFO_DIR}')

    # Default AFL_SYNC_TIME is 30 minutes

    ctx.run(
        f'AFL_FINAL_SYNC=1 '  # still needs afl-cmin
        f'{AFL.FUZZ} '
        f'-i {AFL.SEED_DIR} '
        f'-o {AFL.OUTPUT_DIR} '
        f'-s 42 '  # seed
        f'-V 10 '  # fuzz for x seconds
        f'-M fuzzer01 '  # main fuzzer
        f'{FUZZ.BIN} '
        f'> {FUZZ.INFO_DIR}/fuzz01.txt 2>&1 '
        f'& '  # background process
    )


@task
def run_fuzzer_mp(ctx: Context):
    ctx.run(f'mkdir -p {FUZZ.INFO_DIR}')

    # Default AFL_SYNC_TIME is 30 minutes

    ctx.run(
        f'AFL_FINAL_SYNC=1 '  # still needs afl-cmin
        f'{AFL.FUZZ} '
        f'-i {AFL.SEED_DIR} '
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
            f'-i {AFL.SEED_DIR} '
            f'-o {AFL.OUTPUT_DIR} '
            f'-s 42 '  # seed
            f'-V 10 '  # fuzz for x seconds
            f'-S fuzzer{core_id_str} '  # secondary fuzzer
            f'{FUZZ.BIN} '
            f'> {FUZZ.INFO_DIR}/fuzz{core_id_str}.txt 2>&1 '
            f'& '  # background process
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
        f'{AFL.CORPUS_DIR}/* '
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


def main():
    pass


if __name__ == '__main__':
    main()
