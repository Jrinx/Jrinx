#!/bin/bash

set -e

[ ! -v COMPILE_MODE ] && COMPILE_MODE=debug
[ ! -v BOARD ] && BOARD=virt

test_name=''
verbose='n'

while [[ $# -gt 0 ]]; do
  case $1 in
  -t|--test)
    if [ $# -lt 2 ]; then
      echo 'Expect test name following -t/--test'
      exit 1
    fi
    shift
    test_name="$1"
    shift
    ;;
  -v|--verbose)
    verbose='y'
    shift
    ;;
  *)
    echo "Unknown option $1"
    exit 1
    ;;
  esac
done

source "${0%/*}"/bash/utils.bash

prefix="[ $COMPILE_MODE, $BOARD ]"

r=0

# shellcheck disable=2086
make $COMPILE_MODE COLOR=n > /dev/null || r=$?
if [ "$r" -ne 0 ]; then
  bfatal "$prefix failed to build Jrinx"
  exit $r
fi

make sbi-fw > /dev/null || r=$?
if [ "$r" -ne 0 ]; then
  bfatal "$prefix failed to build opensbi firmware"
  exit $r
fi

test_list="$test_name"

if [ -z "$test_name" ]; then
  test_list=$(
    find tests-conf/* -maxdepth 1 -name '*-test.json' | \
    sort | \
    sed -rne 's/[^\/]+\/([a-zA-Z\-]+-test)\.json/\1/p'
  )
else
  test_list="$test_name"
fi

for test_case in $test_list; do
  bgreen "$prefix run    $test_case"
  conf_file="tests-conf/$test_case.json"
  if [ "$verbose" = 'y' ]; then
    scripts/judge "$conf_file" || r=$?
  else
    scripts/judge "$conf_file" -sn 2> /dev/null || r=$?
  fi
  if [ "$r" -ne 0 ]; then
    bfatal "$prefix failed $test_case"
    exit $r
  else
    bgreen "$prefix passed $test_case"
  fi
done
