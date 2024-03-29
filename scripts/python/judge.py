#!/usr/bin/env python3

from __future__ import annotations
from abc import abstractmethod
import argparse
from datetime import datetime
import os
import pathlib
import psutil
import re
import signal
import subprocess

import yaml

from utils import *
from sysconf import encode_conf

try:
    from yaml import CLoader as Loader
except ImportError:
    from yaml import Loader

CHILD_TIMEOUT = 60
CHILD_FINAL_STUCK_TIMEOUT = 5
CHILD_TERM_TIMEOUT = 5
LOG_PATH = 'logs'


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
    def __call__(self, s: str) -> tuple[bool, tuple[str]]:  # Retire, Picked-Strings
        pass

    @abstractmethod
    def __str__(self) -> str:
        pass

    def __repr__(self) -> str:
        return self.__str__()

    @classmethod
    def of(cls, pattern) -> Pattern:
        match pattern:
            case str():
                return StringPattern(pattern)
            case {'type': typ, **__}:
                return eval(f'{typ.title()}Pattern')(pattern)
            case _:
                raise PatternTypeError(f'Unsupported pattern type {type(pattern)}')


class StringPattern(Pattern):
    def __init__(self, pattern) -> None:
        super().__init__(pattern)
        self.__regex: re.Pattern = re.compile(pattern)

    def __str__(self) -> str:
        return f'(re: {self.__regex})'

    def __call__(self, s: str) -> tuple[bool, tuple[str]]:
        mat = self.__regex.search(s)
        if mat:
            return True, (mat[0],)
        return False, ()


class OrderedPattern(Pattern):
    def __init__(self, pattern) -> None:
        super().__init__(pattern)
        self.__vals = tuple(Pattern.of(p) for p in pattern['vals'])
        self.__next = 0

    def __str__(self) -> str:
        return f'(ordered: {self.__vals})'

    def __call__(self, s: str) -> tuple[bool, tuple[str]]:
        retire, pick = self.__vals[self.__next](s)
        if pick:
            if retire:
                self.__next += 1
            return self.__next >= len(self.__vals), pick
        return False, ()


class UnorderedPattern(Pattern):
    def __init__(self, pattern) -> None:
        super().__init__(pattern)
        self.__wait = set(Pattern.of(p) for p in pattern['vals'])
        self.__all = frozenset(self.__wait)

    def __str__(self) -> str:
        return f'(unordered: {self.__all})'

    def __call__(self, s: str) -> tuple[bool, tuple[str]]:
        rmset = set()
        all_pick = []
        for p in self.__wait:
            retire, pick = p(s)
            if pick:
                all_pick += pick
                if retire:
                    rmset.add(p)
        self.__wait -= rmset
        return len(self.__wait) == 0, tuple(all_pick)


class RepeatedPattern(Pattern):
    def __init__(self, pattern) -> None:
        super().__init__(pattern)
        self.__origin_pat = pattern['pat']
        self.__pat = Pattern.of(self.__origin_pat)
        self.__count = int(pattern['count'])
        self.__rem = self.__count

    def __str__(self) -> str:
        return f'(repeated * {self.__count}: {self.__pat})'

    def __call__(self, s: str) -> tuple[bool, tuple[str]]:
        retire, pick = self.__pat(s)
        if pick:
            if retire:
                self.__rem -= 1
                self.__pat = Pattern.of(self.__origin_pat)
        return self.__rem == 0, pick


class Rules:
    def __init__(self, verbose: bool, conf) -> None:
        self.__verbose = verbose
        self.__expected_patterns = Pattern.of(conf['expected'])
        if conf.get('unexpected'):
            self.__unexpected_patterns = Pattern.of(conf['unexpected'])
        else:
            self.__unexpected_patterns = None
        self.__done = False

    def __call__(self, line: str) -> bool:
        if self.__unexpected_patterns:
            retire, pick = self.__unexpected_patterns(line)
            if pick:
                if retire:
                    if self.__verbose:
                        fatal(f'detecting unexpected pattern \
"{self.__unexpected_patterns}"', file=sys.stdout)
                    raise RulesNotSatisified('unexpected pattern detected')
        if not self.__done:
            retire, pick = self.__expected_patterns(line)
            if pick:
                if self.__verbose:
                    info(f'picking expected string {pick}')
                if retire:
                    if self.__verbose:
                        info(f'detecting expected pattern "{self.__expected_patterns}"')
            self.__done = retire
            if self.__done:
                return True
        return False

    def __enter__(self, *_):
        return self

    def __exit__(self, *_):
        if not self.__done:
            raise RulesNotSatisified('missing expected pattern')


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
        conf = yaml.load(rf, Loader=Loader)
    extends_rules_file = conf.pop('extends', None)
    while extends_rules_file:
        extends_rules_path = os.path.join(os.path.dirname(
            os.path.abspath(args.rules_file)), extends_rules_file)
        if not os.path.isfile(extends_rules_path):
            raise FileNotFoundError(extends_rules_path)
        with open(extends_rules_path, 'r', encoding='utf-8') as erf:
            extended_conf = yaml.load(erf, Loader=Loader)
        for key in extended_conf.keys():
            if key not in conf.keys():
                conf[key] = extended_conf[key]
        extends_rules_file = conf.pop('extends', None)
    sysconf_file = conf.get('sysconf')
    if sysconf_file:
        sysconf_path = pathlib.Path(os.path.dirname(os.path.abspath(
            args.rules_file))).parent / 'sys-conf' / sysconf_file
        if not os.path.isfile(sysconf_path):
            raise FileNotFoundError(sysconf_path)
        with open(sysconf_path, 'r', encoding='utf-8') as sf:
            sysconf_conf = yaml.load(sf, Loader=Loader)
        os.environ['ARGS'] = os.environ.get('ARGS', '') + ' ' + encode_conf(sysconf_conf)
    bootargs = conf.get('bootargs')
    if bootargs:
        os.environ['ARGS'] = os.environ.get('ARGS', '') + ' ' + bootargs

    cmd = ['make', 'run']
    interactive = conf.get('interactive')
    if interactive:
        interactive = [{'when': Pattern.of(i['when']), 'send': i['send']} for i in interactive]
    ch = subprocess.Popen(cmd,
                          stdin=subprocess.PIPE if interactive else subprocess.DEVNULL,
                          stdout=subprocess.PIPE,
                          stderr=subprocess.STDOUT,
                          start_new_session=True,
                          )

    ch_out = []

    try:
        with Rules(verbose, conf) as rules_check:
            final_stuck = False

            def on_timeout(*_):
                if ch:
                    eliminate_child(ch, verbose and not suppress_emu_out)
                if final_stuck:
                    message = f'Command {cmd} stucked {CHILD_FINAL_STUCK_TIMEOUT} seconds \
after expected patterns detected'
                else:
                    message = f'Command {cmd} timed out after {CHILD_TIMEOUT} seconds'
                raise JudgeTimeout(message)

            signal.signal(signal.SIGALRM, on_timeout)
            signal.alarm(CHILD_TIMEOUT)
            for out in ch.stdout:
                line = out if isinstance(out, str) else out.decode('utf-8')
                ch_out.append(line)
                if verbose and not suppress_emu_out:
                    sys.stdout.write(line)
                if interactive:
                    retire, _ = interactive[0]['when'](line)
                    if retire:
                        ch.stdin.write(interactive[0]['send'].encode('utf-8'))
                        ch.stdin.flush()
                        interactive = interactive[1:]
                if final_stuck and len(line) != 0:
                    rules_check(line)
                if not final_stuck and rules_check(line):
                    final_stuck = True
                    signal.alarm(CHILD_FINAL_STUCK_TIMEOUT)
    except JudgeException as e:
        if not verbose:
            now = datetime.now().strftime('%Y-%m-%d_%H-%M-%S')
            if not os.path.isdir(LOG_PATH):
                os.mkdir(LOG_PATH)
            log_file_name = f'{os.path.basename(args.rules_file).split(".")[0]}-{now}.log'
            log_file_path = pathlib.Path(LOG_PATH) / log_file_name
            with open(log_file_path, 'w', encoding='utf-8') as lf:
                lf.writelines(ch_out)
        fatal(e)
        raise e
    finally:
        signal.alarm(0)
        if ch:
            eliminate_child(ch, verbose and not suppress_emu_out)
        if verbose:
            info('Judge end')


if __name__ == '__main__':
    main()
