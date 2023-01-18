#!/usr/bin/python3

from __future__ import annotations
from abc import abstractmethod
import argparse
import json
import os
import psutil
import signal
import subprocess
from typing import Sequence

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


class PatternTypeError(JudgeException):
    def __init__(self, message: str) -> None:
        super().__init__('Pattern Type Error', message)


class Pattern:
    def __init__(self, pattern) -> None:
        self._pattern = pattern

    @abstractmethod
    def __call__(self, s: str) -> tuple[bool, bool, str]:  # Matched, Picked, PickedStr
        pass

    def __str__(self) -> str:
        return str(self._pattern)

    def __repr__(self) -> str:
        return self.__str__()

    @classmethod
    def of(cls, pattern) -> Pattern:
        match pattern:
            case str():
                return StringPattern(pattern)
            case tuple() | list():
                return SeqPattern(pattern)
            case _:
                raise PatternTypeError(f'Unsupported pattern type {type(pattern)}')


class StringPattern(Pattern):
    def __call__(self, s: str) -> tuple[bool, bool, str]:
        if self._pattern in s:
            return True, True, s
        return False, False, None


class SeqPattern(Pattern):
    def __init__(self, pattern: Sequence[str]) -> None:
        super().__init__(pattern)
        self.__next = 0

    def __call__(self, s: str) -> tuple[bool, bool, str]:
        pattern = self._pattern[self.__next]
        if pattern in s:
            self.__next += 1
            return self.__next >= len(self._pattern), True, pattern
        return False, False, None


class Rules:
    def __init__(self, verbose: bool, conf) -> None:
        self.__verbose = verbose
        self.__unexpected = frozenset(conf['unexpected'])
        self.__patterns = set(Pattern.of(e) for e in conf['expected'])
        self.__expected = frozenset(self.__patterns)

    def __str__(self) -> str:
        return f'expected: {self.__expected}, unexpected: {self.__unexpected}'

    def __call__(self, line: str) -> int:
        for s in self.__unexpected:
            if s in line:
                if self.__verbose:
                    fatal(f'detecting unexpected pattern "{s}"', file=sys.stdout)
                raise RulesNotSatisified(f'unexpected pattern detected')
        rmset = set()
        for pattern in self.__patterns:
            matched, picked, picked_str = pattern(line)
            if picked:
                if self.__verbose:
                    info(f'picking string "{picked_str}"')
                if matched:
                    if self.__verbose:
                        info(f'detecting expected pattern "{str(pattern)}"')
                    rmset.add(pattern)
                break
        self.__patterns -= rmset
        return len(self.__patterns)

    def __enter__(self, *_):
        return self

    def __exit__(self, *_):
        if len(self.__patterns) != 0:
            raise RulesNotSatisified(f'missing expected pattern {str(self.__patterns)}')


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

    if not os.path.isfile(args.rules_file):
        raise FileNotFoundError(args.rules_file)
    with open(args.rules_file, 'r', encoding='utf-8') as rf:
        conf = json.load(rf)
    extends_rules_file = conf.pop('extends', None)
    while extends_rules_file:
        extends_rules_path = os.path.join(os.path.dirname(args.rules_file), extends_rules_file)
        if not os.path.isfile(extends_rules_path):
            raise FileNotFoundError(extends_rules_path)
        with open(extends_rules_path, 'r', encoding='utf-8') as erf:
            extended_conf = json.load(erf)
        for key in extended_conf.keys():
            if key not in conf.keys():
                conf[key] = extended_conf[key]
        extends_rules_file = conf.pop('extends', None)

    cmd = ['make', 'run']
    interactive = conf.get('interactive')
    ch = subprocess.Popen(cmd,
                          stdin=subprocess.PIPE if interactive else None,
                          stdout=subprocess.PIPE,
                          stderr=subprocess.STDOUT,
                          start_new_session=True,
                          )

    try:
        with Rules(verbose, conf) as rules_check:
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
                    sys.stdout.write(line)
                if interactive and interactive[0]['when'] in line:
                    ch.stdin.write(interactive[0]['send'].encode('utf-8'))
                    ch.stdin.flush()
                    interactive = interactive[1:]
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
