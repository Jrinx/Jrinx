#!/usr/bin/python3

import argparse
import os
import subprocess
from collections import OrderedDict
from typing import Callable

import pathos.multiprocessing as mp

from utils import *


PARALLEL_POOL_SIZE = mp.cpu_count()
JRINX_MINTICK = 100


def run_test(test, /, *,
             enter: Callable[[str], None],
             success_hook: Callable[[str], None],
             failure_hook: Callable[[str], None]):
    enter(test)
    cmd = ['scripts/judge', os.path.join('tests-conf', test + '.yml')]
    if not verbose:
        cmd.append('-sn')
    try:
        if verbose:
            subprocess.check_call(cmd,
                                  env=dict(os.environ, BOARD=board),
                                  )
        else:
            subprocess.check_call(cmd,
                                  env=dict(os.environ, BOARD=board),
                                  stderr=subprocess.DEVNULL,
                                  )
    except subprocess.CalledProcessError as e:
        failure_hook(test)
        return e.returncode
    else:
        success_hook(test)
        return 0


def run_testset(testset):
    binfo(f'Run tests in {compile_mode} mode on {board} board')

    def run(test):
        return run_test(test,
                        enter=lambda t: binfo(f'run    {t}'),
                        success_hook=lambda t: binfo(f'passed {t}'),
                        failure_hook=lambda t: bfatal(f'failed {t}'),
                        )
    if len(testset) > 1 and parallel:
        res = mp.ThreadingPool(PARALLEL_POOL_SIZE).map(run, testset)
    else:
        res = [run(test) for test in testset]

    failed_testset = tuple(map(lambda t: t[0], filter(lambda t: t[1], zip(testset, res))))
    if failed_testset:
        bfatal(f'Failed tests: {failed_testset}')

    return res


def run_testset_rich(testset):
    from rich.align import Align
    from rich.console import Console
    from rich.live import Live
    from rich.table import Table
    from rich.text import Text

    test_status = OrderedDict((test, 'waiting') for test in testset)

    def gen_table() -> Table:
        table = Table('Test', 'Status')
        table.title = Text(f'{compile_mode} mode on {board} board', style='bold')
        for test, status in test_status.items():
            status = Text(status,
                          style='white' if status == 'waiting'
                          else 'yellow' if status == 'running'
                          else 'green' if status == 'passed' else 'red')
            table.add_row(test, status)
        table = Align.center(table)
        return table

    console = Console()
    console.clear()
    with Live(gen_table(), console=console, screen=False, refresh_per_second=1) as live:
        def run(test):
            def enter(test):
                test_status[test] = 'running'
                live.update(gen_table())

            def success_hook(test):
                test_status[test] = 'passed'
                live.update(gen_table())

            def failure_hook(test):
                test_status[test] = 'failed'
                live.update(gen_table())

            return run_test(test,
                            enter=enter,
                            success_hook=success_hook,
                            failure_hook=failure_hook,
                            )
        if len(testset) > 1 and parallel:
            res = mp.ThreadingPool(PARALLEL_POOL_SIZE).map(run, testset)
        else:
            res = [run(test) for test in testset]

    return res


def main():
    args = argparse.ArgumentParser()
    args.add_argument('-t', '--test')
    args.add_argument('-v', '--verbose', action='store_true')
    args.add_argument('-p', '--parallel', action='store_true')
    args.add_argument('-n', '--no-make', action='store_true')
    args.add_argument('-r', '--rich', action='store_true')
    args = args.parse_args()
    test = args.test

    global verbose, parallel, no_make, rich, compile_mode, board
    verbose = args.verbose
    parallel = args.parallel
    no_make = args.no_make
    rich = args.rich

    if rich and verbose:
        raise ValueError('Cannot use both rich and verbose mode')

    try:
        compile_mode = os.environ['COMPILE_MODE']
    except KeyError:
        compile_mode = 'debug'

    try:
        board = os.environ['BOARD']
    except KeyError:
        board = 'virt'

    if test:
        if os.path.isfile(os.path.join('tests-conf', test + '.yml')):
            testset = frozenset((test,))
        else:
            raise FileNotFoundError(f'No such test: {test}')
    else:
        testset = frozenset(f for f in os.listdir('tests-conf') if f.endswith('-test.yml'))
    testset = tuple(sorted(os.path.splitext(f)[0] for f in testset))

    if not no_make:
        try:
            subprocess.check_call(['make', compile_mode],
                                  env=dict(os.environ, COLOR='n', MINTICK=f'{JRINX_MINTICK}'),
                                  stdout=subprocess.DEVNULL,
                                  )
        except subprocess.CalledProcessError as e:
            bfatal('failed to build jrinx')
            return e.returncode
        try:
            subprocess.check_call(['make', 'sbi-fw'],
                                  stdout=subprocess.DEVNULL,
                                  stderr=subprocess.DEVNULL,
                                  )
        except subprocess.CalledProcessError as e:
            bfatal('failed to build opensbi firmware')
            return e.returncode

    if rich:
        res = run_testset_rich(testset)
    else:
        res = run_testset(testset)

    return any(res)


if __name__ == '__main__':
    exit(main())
