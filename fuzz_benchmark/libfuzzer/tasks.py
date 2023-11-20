import json
import subprocess

from invoke import task, Context


# https://github.com/epi052/fuzzing-101-solutions/blob/main/exercise-5/tasks.py

# @task
# def get_lines_covered(ctx: Context):
#     ctx.run('llvm-cov export ./fuzzer_coverage -instr-profile=fuzzer.profdata -format=text -summary-only > h.json')

def get_lines_covered():
    cmd = ['llvm-cov', 'export', './fuzzer_coverage', '-instr-profile=fuzzer.profdata', '-format=text', '-summary-only']
    out = subprocess.check_output(cmd)

    parsed_json = json.loads(out)
    totals_lines = parsed_json['data'][0]['totals']['lines']

    count = totals_lines['count']
    count_covered = totals_lines['covered']
    percent = totals_lines['percent']

    return count, count_covered, percent


def main():
    stats = get_lines_covered()
    print(stats)


if __name__ == '__main__':
    main()
