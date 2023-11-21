import pathlib

from invoke import task, Context

THIS_SCRIPT_DIR = pathlib.Path(__file__).parent.resolve()

BUILD_DIR = THIS_SCRIPT_DIR / 'libxml2'


@task
def build_libxml2(ctx: Context):
    ctx.run(
        f'git clone '
        f'--branch v2.12.0 '
        f'--depth 1 '
        f'https://gitlab.gnome.org/GNOME/libxml2 {BUILD_DIR} '
    )

    with ctx.cd(BUILD_DIR):
        ctx.run(
            './autogen.sh '
            '--disable-shared '
            '--without-debug '
            '--without-ftp '
            '--without-http '
            '--without-legacy '
            '--without-python '
            'make -j$(nproc) '
        )

        ctx.run(f'make fuzz.o')
