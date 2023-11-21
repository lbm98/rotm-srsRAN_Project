import pathlib

from invoke import Context

THIS_SCRIPT_DIR = pathlib.Path(__file__).parent.resolve()

HARNESS_CPP = THIS_SCRIPT_DIR / 'target.cpp'
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

    if not source_dir.exists():
        ctx.run(
            f'git clone '
            f'--branch v2.12.0 '
            f'--depth 1 '
            f'https://gitlab.gnome.org/GNOME/libxml2.git/ '
            f'{source_dir} '
        )

    with ctx.cd(source_dir):
        ctx.run(
            './autogen.sh '
            '--disable-shared '
            '--without-debug '
            '--without-ftp '
            '--without-http '
            '--without-legacy '
            '--without-python '
        )

        ctx.run(
            f'CC={cc} '
            f'CXX={cxx} '
            f'CFLAGS="{cflags}" '
            f'CXXFLAGS="{cxxflags}" '
        )
