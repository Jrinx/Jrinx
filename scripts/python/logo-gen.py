#!/usr/bin/python3

import argparse


def main():
    args = argparse.ArgumentParser()
    args.add_argument('logo_path')
    args = args.parse_args()
    with open(args.logo_path, 'r', encoding='utf-8') as logo_file:
        logo = logo_file.read()
    content = ','.join([f'0x{int(b):x}' for b in bytes(logo, 'utf-8')])
    print('{' + content + '}')


if __name__ == '__main__':
    main()
