import pathlib

from invoke import Context

THIS_SCRIPT_DIR = pathlib.Path(__file__).parent.resolve()

HARNESS_CPP = THIS_SCRIPT_DIR / 'target.cc'
STANDALONE_FUZZ_TARGET_MAIN_C = THIS_SCRIPT_DIR / 'StandaloneFuzzTargetMain.c'
STANDALONE_FUZZ_TARGET_MAIN_O = THIS_SCRIPT_DIR / 'StandaloneFuzzTargetMain.o'


def build_lib(
        ctx: Context,
        cc: pathlib.Path,
        cxx: pathlib.Path,
        cflags: str,
        cxxflags: str,
        build_dir: pathlib.Path,
        install_dir: pathlib.Path,
        lib_dir: pathlib.Path,
        seed_dir: pathlib.Path | None,
):
    if build_dir.exists() or lib_dir.exists():
        print('libraries already build...')
        return

    ctx.run(
        f'git clone '
        f'--branch v2.12.0 '
        f'--depth 1 '
        f'https://gitlab.gnome.org/GNOME/libxml2.git/ '
        f'{build_dir} '
    )

    with ctx.cd(build_dir):
        ctx.run(
            f'CC={cc} '
            f'CFLAGS="{cflags}" '
            f'./autogen.sh '
            f'--disable-shared '
            f'--without-debug '
            f'--without-ftp '
            f'--without-http '
            f'--without-legacy '
            f'--without-python '
            f'--prefix {install_dir} '
        )

        ctx.run('make -j$(nproc)')
        ctx.run('make install')

        ctx.run(f'mkdir {lib_dir}')
        ctx.run(f'cp {install_dir}/lib/*.a {lib_dir}')

        if seed_dir is not None:
            ctx.run('make seed/xml.stamp')
            ctx.run('zip -j $OUT/xml_seed_corpus.zip seed/xml/*')


def build_bin(
        ctx: Context,
        cc: pathlib.Path,
        cxx: pathlib.Path,
        cflags: str,
        cxxflags: str,
        install_dir: pathlib.Path,
        lib_dir: pathlib.Path,
        bin_dir: pathlib.Path,
        bin: pathlib.Path,
        insert_main: bool
):
    if bin_dir.exists():
        print('binary already build...')
        return

    ctx.run(f'mkdir -p {bin_dir}')

    if insert_main:
        ctx.run(f'{cc} {STANDALONE_FUZZ_TARGET_MAIN_C} -c -o {STANDALONE_FUZZ_TARGET_MAIN_O}')
        optional_main = STANDALONE_FUZZ_TARGET_MAIN_O
    else:
        optional_main = ''

    ctx.run(
        f'{cxx} '
        f'{optional_main} '
        f'{HARNESS_CPP} '
        f'{cxxflags} '
        f'-fuse-ld=lld '
        f'{lib_dir}/*.a '
        f'-I {install_dir}/include/libxml2 '
        f'-Wl,-Bstatic -lz -llzma -Wl,-Bdynamic '
        f'-o {bin} '
    )