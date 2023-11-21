import pathlib

from invoke import Context

THIS_SCRIPT_DIR = pathlib.Path(__file__).parent.resolve()
CMAKE_SOURCE_DIR = THIS_SCRIPT_DIR.parent.parent.parent.resolve()

HARNESS_CPP = THIS_SCRIPT_DIR / 'harness.cpp'
STANDALONE_FUZZ_TARGET_MAIN_C = THIS_SCRIPT_DIR / 'StandaloneFuzzTargetMain.c'
STANDALONE_FUZZ_TARGET_MAIN_O = THIS_SCRIPT_DIR / 'StandaloneFuzzTargetMain.o'


def build_lib(
        ctx: Context,
        cc: pathlib.Path,
        cxx: pathlib.Path,
        cflags: str,
        cxxflags: str,
        source_dir: pathlib.Path,
        build_dir: pathlib.Path,
        lib_dir: pathlib.Path,
):
    if build_dir.exists() or lib_dir.exists():
        print('libraries already build...')
        return

    ctx.run(f'mkdir -p {build_dir}')

    with ctx.cd(build_dir):
        ctx.run(
            f'cmake '
            f'-G Ninja '
            f'-DCMAKE_C_COMPILER="{cc}" '
            f'-DCMAKE_CXX_COMPILER="{cxx}" '
            f'-DCMAKE_C_FLAGS="{cflags}" '
            f'-DCMAKE_CXX_FLAGS="{cxxflags}" '
            f'-DCMAKE_LIBRARY_OUTPUT_DIRECTORY="{lib_dir}" '
            f'-DCMAKE_ARCHIVE_OUTPUT_DIRECTORY="{lib_dir}" '
            f'{CMAKE_SOURCE_DIR} '
        )

        ctx.run('cmake --build . --target rrc_ue_test_helpers srsran_rrc -v')


def internal_build_bin(
        ctx: Context,
        cc: pathlib.Path,
        cxx: pathlib.Path,
        cflags: str,
        cxxflags: str,
        lib_dir: pathlib.Path,
        bin_dir: pathlib.Path,
        bin: pathlib.Path,
        optional_main: pathlib.Path | str
):
    if bin_dir.exists():
        print('binary already build...')
        return

    ctx.run(f'mkdir -p {bin_dir}')

    ctx.run(
        f'{cxx} '
        f'{optional_main} '
        f'{HARNESS_CPP} '
        f'{cxxflags} '
        f'-fuse-ld=lld '
        f'-I {CMAKE_SOURCE_DIR} '
        f'-I {CMAKE_SOURCE_DIR}/include '
        f'-I {CMAKE_SOURCE_DIR}/external '
        f'-I {CMAKE_SOURCE_DIR}/external/fmt/include '
        f'{lib_dir}/*.a '
        f'-lgtest '
        f'-lmbedtls -lmbedx509 -lmbedcrypto '
        f'-o {bin} '
    )


def build_bin(
        ctx: Context,
        cc: pathlib.Path,
        cxx: pathlib.Path,
        cflags: str,
        cxxflags: str,
        lib_dir: pathlib.Path,
        bin_dir: pathlib.Path,
        bin: pathlib.Path
):
    internal_build_bin(
        ctx=ctx,
        cc=cc,
        cxx=cxx,
        cflags=cflags,
        cxxflags=cxxflags,
        lib_dir=lib_dir,
        bin_dir=bin_dir,
        bin=bin,
        optional_main=''
    )


def build_coverage_bin(
        ctx: Context,
        cc: pathlib.Path,
        cxx: pathlib.Path,
        cflags: str,
        cxxflags: str,
        lib_dir: pathlib.Path,
        bin_dir: pathlib.Path,
        bin: pathlib.Path
):
    ctx.run(f'{cc} {STANDALONE_FUZZ_TARGET_MAIN_C} -c -o {STANDALONE_FUZZ_TARGET_MAIN_O}')

    internal_build_bin(
        ctx=ctx,
        cc=cc,
        cxx=cxx,
        cflags=cflags,
        cxxflags=cxxflags,
        lib_dir=lib_dir,
        bin_dir=bin_dir,
        bin=bin,
        optional_main=STANDALONE_FUZZ_TARGET_MAIN_O
    )


def main():
    pass


if __name__ == '__main__':
    main()
