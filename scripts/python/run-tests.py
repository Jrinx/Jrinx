#!/usr/bin/python3

import argparse
import multiprocessing
import os
import subprocess

from utils import *


PARALLEL_POOL_SIZE = multiprocessing.cpu_count() // 5  # Qemu runs smp on 5 cores


def run_test(test):
    binfo(f'run    {test}')
    cmd = ['scripts/judge', os.path.join('tests-conf', test + '.json')]
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
        bfatal(f'failed {test}')
        return e.returncode
    else:
        binfo(f'passed {test}')


def run_testset(testset):
    try:
        subprocess.check_call(['make', compile_mode],
                              env=dict(os.environ, COLOR='n'),
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
    if len(testset) > 1 and parallel:
        return multiprocessing.Pool(PARALLEL_POOL_SIZE).map(run_test, testset)
    else:
        return map(run_test, testset)


def main():
    args = argparse.ArgumentParser()
    args.add_argument('-t', '--test')
    args.add_argument('-v', '--verbose', action='store_true')
    args.add_argument('-p', '--parallel', action='store_true')
    args = args.parse_args()
    test = args.test

    global verbose, parallel, compile_mode, board
    verbose = args.verbose
    parallel = args.parallel

    if verbose and parallel:
        raise ValueError('-p/--parallel and -v/--verbose are mutually exclusive')

    try:
        compile_mode = os.environ['COMPILE_MODE']
    except KeyError:
        compile_mode = 'debug'

    try:
        board = os.environ['BOARD']
    except KeyError:
        board = 'virt'

    binfo(f'Run tests in {compile_mode} mode on {board} board')

    if test:
        if os.path.isfile(os.path.join('tests-conf', test + '.json')):
            testset = frozenset((test,))
        else:
            raise FileNotFoundError(f'No such test: {test}')
    else:
        testset = frozenset(f for f in os.listdir('tests-conf') if f.endswith('-test.json'))
    testset = tuple(sorted(os.path.splitext(f)[0] for f in testset))

    res = run_testset(testset)
    failed_testset = tuple(map(lambda t: t[0], filter(lambda t: t[1], zip(testset, res))))
    if failed_testset:
        bfatal(f'Failed tests: {failed_testset}')
    return any(res)


if __name__ == '__main__':
    exit(main())
