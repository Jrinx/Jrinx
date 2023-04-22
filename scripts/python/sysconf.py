#!/usr/bin/python3

import argparse
import os

import yaml

try:
    from yaml import CLoader as Loader
except ImportError:
    from yaml import Loader


ITEM_ENCODING_FUNC = {
    'memory': lambda n: eval(str(n)),
    'max-msg-size': lambda n: eval(str(n)),
    'ports': '&'.join,
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
    if conf.get('queuing-ports'):
        res += ['--qp-conf']
        ports_conf = []
        for port in conf['queuing-ports']:
            ports_conf += [','.join(encode_item(k, v) for k, v in port.items())]
        res += [';'.join(ports_conf)]
    if conf.get('channel'):
        res += ['--ch-conf']
        chan_conf = []
        for chan in conf['channel']:
            chan_conf += [','.join(encode_item(k, v) for k, v in chan.items())]
        res += [';'.join(chan_conf)]
    return ' '.join(res)


def main():
    args = argparse.ArgumentParser()
    args.add_argument('conf_path')
    args = args.parse_args()
    conf_path = args.conf_path
    if not os.path.isfile(conf_path):
        raise FileNotFoundError(conf_path)
    with open(conf_path, 'r', encoding='utf-8') as conf_file:
        conf_content = yaml.load(conf_file, Loader=Loader)
    out_content = encode_conf(conf_content)
    print(out_content)


if __name__ == '__main__':
    main()
