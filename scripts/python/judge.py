#!/usr/bin/python3

import argparse
import psutil
import re
import signal
import subprocess

from utils import *

CHILD_TIMEOUT = 10
CHILD_TERM_TIMEOUT = 5


class PatternNotMatchError(Exception):
    pass


def eliminate_child(ch, verbose):
    try:
        proc = psutil.Process(ch.pid)
    except psutil.NoSuchProcess:
        return

    if ch.stdin:
        try:
            ch.stdin.close()
        except RuntimeError:
            pass

    if ch.stdout:
        try:
            if verbose:
                os.set_blocking(ch.stdout.fileno(), False)
                b = ch.stdout.read()
                if b:
                    sys.stdout.buffer.write(b)
                    sys.stdout.flush()
            ch.stdout.close()

        except RuntimeError:
            pass

    chs = [proc]
    chs.extend(proc.children(recursive=True))

    alive = []
    for p in chs:
        try:
            p.terminate()
            alive.append(p)
        except psutil.NoSuchProcess:
            pass

    _, alive = psutil.wait_procs(alive, timeout=CHILD_TERM_TIMEOUT)
    for p in alive:
        try:
            warn(f'Killing process {p.pid} ({p.name()})')
            p.kill()
        except psutil.NoSuchProcess:
            pass


def check_output(lines, verbose):
    def match_pattern(s) -> bool:
        PATTERN = re.compile(r'^\[.+?hart#\d\s*\].+?arg-driven test done, halt!$')
        mat = PATTERN.match(s)
        if mat is not None:
            if verbose:
                info(f'Pattern found on "{s}"')
            return True
        return False

    if not any(map(match_pattern, lines)):
        if verbose:
            fatal('Failed to match pattern')
        raise PatternNotMatchError()


def main():
    args = argparse.ArgumentParser()
    args.add_argument('-n', '--suppress-emu-out', action='store_true')
    args.add_argument('-s', '--silent', action='store_true')
    args = args.parse_args()

    suppress_emu_out = args.suppress_emu_out
    verbose = not args.silent
    if verbose:
        info(f'Judge begin with args {args}')

    cmd = ['make', 'run']
    ch = subprocess.Popen(cmd,
                          stdin=None,
                          stdout=subprocess.PIPE,
                          stderr=subprocess.STDOUT,
                          start_new_session=True,
                          )

    def on_timeout(*_):
        if ch:
            warn(f'{cmd} timed out after {CHILD_TIMEOUT} seconds')
            eliminate_child(ch, verbose)

    signal.signal(signal.SIGALRM, on_timeout)
    signal.alarm(CHILD_TIMEOUT)

    ch_out = ''

    try:
        for out in ch.stdout:
            if verbose and not suppress_emu_out:
                sys.stdout.buffer.write(out)
                sys.stdout.flush()
            ch_out += out.decode('utf-8')
    finally:
        signal.alarm(0)
        if ch:
            eliminate_child(ch, verbose)

    try:
        check_output(ch_out.split('\n'), verbose)
    finally:
        if verbose:
            info('Judge end')


if __name__ == '__main__':
    main()
