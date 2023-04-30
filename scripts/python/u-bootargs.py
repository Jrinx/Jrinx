#!/usr/bin/python3

import argparse
import os
import sys


def main():
    args = argparse.ArgumentParser()
    args.add_argument('-i', '--input')
    args.add_argument('-o', '--output',
                      required=True,
                      help='path to output file (always in tftp root directory)',
                      )

    args = args.parse_args()
    input = args.input
    output = args.output

    if input:
        with open(input, 'r') as f:
            args = f.read()
    else:
        args = sys.stdin.read()

    try:
        extra_args = os.environ['ARGS']
    except KeyError:
        pass
    else:
        args += ' ' + extra_args

    with open(output, 'w') as f:
        f.write(f'bootargs={args}')

    print(f'tftp ${{env_addr_r}} {os.path.basename(output)}; \
env import -t ${{env_addr_r}} {os.path.getsize(output)}')


if __name__ == '__main__':
    main()
