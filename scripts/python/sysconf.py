#!/usr/bin/python3

import argparse
import json
import os


ITEM_ENCODING_FUNC = {
    'memory': eval,
}


def encode_item(key, val) -> str:
    val = ITEM_ENCODING_FUNC.get(key, str)(val)
    return f'{key}={val}'


def encode_conf(conf: dict) -> str:
    res = []

    if conf.get('partitions'):
        res += ['--pa-conf']
        parts_conf = []
        for part in conf['partitions']:
            parts_conf += [','.join(encode_item(k, v) for k, v in part.items())]
        res += [';'.join(parts_conf)]
    if conf.get('scheduler'):
        res += ['--sc-conf']
        table_conf = []
        for sched in conf['scheduler']:
            table_conf += [','.join(encode_item(k, v) for k, v in sched.items())]
        res += [';'.join(table_conf)]
    return ' '.join(res)


def main():
    args = argparse.ArgumentParser()
    args.add_argument('conf_path')
    args = args.parse_args()
    conf_path = args.conf_path
    if not os.path.isfile(conf_path):
        raise FileNotFoundError(conf_path)
    with open(conf_path, 'r', encoding='utf-8') as conf_file:
        conf_content = json.load(conf_file)
    out_content = encode_conf(conf_content)
    print(out_content)


if __name__ == '__main__':
    main()
