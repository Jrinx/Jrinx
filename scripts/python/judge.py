#!/usr/bin/python3

import argparse
import json
import psutil
import signal
import subprocess

from utils import *

CHILD_TIMEOUT = 60
CHILD_FINAL_STUCK_TIMEOUT = 5
CHILD_TERM_TIMEOUT = 5


class JudgeException(Exception):
    def __init__(self, title: str, message: str) -> None:
        self.__title = title
        self.__message = message

    def __str__(self) -> str:
        return f'[ Judge Exception ] {self.__title}: {self.__message}'


class RulesNotSatisified(JudgeException):
    def __init__(self, message: str) -> None:
        super().__init__('Rules Not Satisified', message)


class JudgeTimeout(JudgeException):
    def __init__(self, message: str) -> None:
        super().__init__('Judge Timeout', message)


class Rules:
    def __init__(self, verbose: bool, conf) -> None:
        self.__verbose = verbose
        self.__expected = frozenset(conf['expected'])
        self.__unexpected = frozenset(conf['unexpected'])
        self.__waited = set(self.__expected)
        assert len(self.__expected & self.__unexpected) == 0

    def __str__(self) -> str:
        return f'expected: {self.__expected}, unexpected: {self.__unexpected}'

    def __call__(self, line: str) -> int:
        for s in self.__unexpected:
            if s in line:
                if self.__verbose:
                    fatal(f'detecting unexpected string "{s}"', file=sys.stdout)
                raise RulesNotSatisified(f'unexpected strings detected')
        for s in self.__expected:
            if s in line:
                self.__waited.remove(s)
                if self.__verbose:
                    info(f'detecting expected string "{s}"')

        return len(self.__waited)

    def __enter__(self, *_):
        return self

    def __exit__(self, *_):
        if len(self.__waited) != 0:
            raise RulesNotSatisified(f'missing expected strings {self.__waited}')


def eliminate_child(ch: subprocess.Popen[bytes], verbose: bool):
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
            ch.stdout.close()

        except RuntimeError:
            pass

    chs = [proc]
    chs.extend(proc.children(recursive=True))

    for p in chs:
        try:
            p.terminate()
        except psutil.NoSuchProcess:
            pass

    _, alive = psutil.wait_procs(chs, timeout=CHILD_TERM_TIMEOUT)
    for p in alive:
        try:
            warn(f'Killing process {p.pid} ({p.name()})')
            p.kill()
        except psutil.NoSuchProcess:
            pass

    ch.wait()


def main():
    args = argparse.ArgumentParser()
    args.add_argument('rules_file')
    args.add_argument('-n', '--suppress-emu-out', action='store_true')
    args.add_argument('-s', '--silent', action='store_true')
    args = args.parse_args()

    suppress_emu_out = args.suppress_emu_out
    verbose = not args.silent

    if verbose:
        info(f'Judge begin with args {args}')

    if os.path.isfile(args.rules_file):
        with open(args.rules_file, 'r', encoding='utf-8') as rf:
            rules = json.load(rf)
    else:
        raise NotADirectoryError(args.rules_file)

    cmd = ['make', 'run']
    ch = subprocess.Popen(cmd,
                          stdin=None,
                          stdout=subprocess.PIPE,
                          stderr=subprocess.STDOUT,
                          start_new_session=True,
                          )

    try:
        with Rules(verbose, rules) as rules_check:
            final_stuck = False

            def on_timeout(*_):
                if ch:
                    eliminate_child(ch, verbose and not suppress_emu_out)
                if final_stuck:
                    message = f'Command {cmd} stucked {CHILD_FINAL_STUCK_TIMEOUT} seconds \
after all expected strings detected'
                else:
                    message = f'Command {cmd} timed out after {CHILD_TIMEOUT} seconds'
                raise JudgeTimeout(message)

            signal.signal(signal.SIGALRM, on_timeout)
            signal.alarm(CHILD_TIMEOUT)

            if verbose:
                info(str(rules_check))
            for out in ch.stdout:
                line = out if isinstance(out, str) else out.decode('utf-8')
                if verbose and not suppress_emu_out:
                    sys.stdout.buffer.write(out)
                if final_stuck and len(line) != 0:
                    rules_check(line)
                if not final_stuck and rules_check(line) == 0:
                    final_stuck = True
                    signal.alarm(CHILD_FINAL_STUCK_TIMEOUT)
    except JudgeTimeout as e:
        warn(e)
    except RulesNotSatisified as e:
        fatal(e)
        exit(1)
    finally:
        signal.alarm(0)
        if ch:
            eliminate_child(ch, verbose and not suppress_emu_out)
        if verbose:
            info('Judge end')


if __name__ == '__main__':
    main()
