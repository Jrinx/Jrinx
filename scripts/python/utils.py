import os
import sys


if sys.stdout.isatty() or 'GITLAB_CI' in os.environ:
    def make_printer(prefix, level, *, file=sys.stdout):
        def wrapper(s, file=file):
            print(f'{prefix}[ {s} ]\033[0m', file=file)

        return wrapper
else:
    def make_printer(prefix, level, *, file=sys.stdout):
        def wrapper(s, file=file):
            plain = f'[{level}] {s}'
            print(plain, file=file)

        return wrapper


bfatal = make_printer('\033[31m\033[01m', 'FATAL', file=sys.stderr)
fatal = make_printer('\033[31m', 'FATAL', file=sys.stderr)

binfo = make_printer('\033[32m\033[01m', 'INFO')
info = make_printer('\033[32m', 'INFO')

warn = make_printer('\033[33m', 'WARN', file=sys.stderr)
