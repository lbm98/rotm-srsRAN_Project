import os
import pathlib


THIS_SCRIPT_DIR = pathlib.Path(__file__).parent.resolve()
CMAKE_SOURCE_DIR = THIS_SCRIPT_DIR.parent.parent.parent.resolve()


def build(
        cc: pathlib.Path,
        cxx: pathlib.Path,
        lib_cflags: str,
        lib_cxxflags: str,
        bin_cflags: str,
        bin_cxxflags: str,
        build_dir: pathlib.Path,
        lib_dir: pathlib.Path,
        bin_dir: pathlib.Path,
        harness: pathlib.Path
):
    if build_dir.exists() or lib_dir.exists():
        print('libraries already build...')
        return

    build_dir.mkdir()
    os.chdir(build_dir)

    os.system(
        f'cmake '
        f'-G Ninja '
        f'-DCMAKE_C_COMPILER="{cc}" '
        f'-DCMAKE_CXX_COMPILER="{cxx}" '
        f'-DCMAKE_C_FLAGS="{lib_cflags}" '
        f'-DCMAKE_CXX_FLAGS="{lib_cxxflags}" '
        f'-DCMAKE_LIBRARY_OUTPUT_DIRECTORY="{lib_dir}" '
        f'-DCMAKE_ARCHIVE_OUTPUT_DIRECTORY="{lib_dir}" '
        f'{CMAKE_SOURCE_DIR} '
    )

    os.system('cmake --build . --target rrc_ue_test_helpers srsran_rrc -v')

    if bin_dir.exists():
        print('binary already build...')
        return

    bin_dir.mkdir()

    os.system(
        f'{cxx} '
        f'{harness} '
        f'-fuse-ld=lld '
        f'-I {CMAKE_SOURCE_DIR} '
        f'-I {CMAKE_SOURCE_DIR}/include '
        f'-I {CMAKE_SOURCE_DIR}/external '
        f'-I {CMAKE_SOURCE_DIR}/external/fmt/include '
        f'{lib_dir}/*.a '
        f'-lgtest '
        f'-lmbedtls -lmbedx509 -lmbedcrypto '
        f'-o {bin_dir} '
    )


def main():
    p = pathlib.Path('/home/lbm')
    os.chdir(p)
    os.system('touch test.txt')


if __name__ == '__main__':
    main()
