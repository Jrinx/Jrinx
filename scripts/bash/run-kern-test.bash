#!/bin/bash

set -e

[ ! -v COMPILE_MODE ] && COMPILE_MODE=debug

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

prefix="[ $COMPILE_MODE mode ]"

r=0

make $COMPILE_MODE > /dev/null || r=$?
if [ "$r" -ne 0 ]; then
  fatal "$prefix failed to build Jrinx"
  exit $r
fi

make sbi-fw > /dev/null || r=$?
if [ "$r" -ne 0 ]; then
  fatal "$prefix failed to build opensbi firmware"
  exit $r
fi

test_list="$test_name"

if [ -z "$test_name" ]; then
  test_list=$(
    find kern-tests -maxdepth 1 -name '*-test.c' | \
    sort | \
    sed -rne 's/kern-tests\/(.+?-test)\.c/\1/p'
  )
else
  test_list="$test_name"
fi

for test_case in $test_list; do
  green "$prefix run $test_case"
  make TEST="$test_case" COLOR=n > /dev/null || r=$?
  if [ "$r" -ne 0 ]; then
    fatal "$prefix failed to build Jrinx with $test_case"
    exit $r
  fi
  conf_file='kern-tests/default-conf.json'
  if [ -f "kern-tests/$test_case-conf.json" ]; then
    conf_file="kern-tests/$test_case-conf.json"
  fi
  if [ "$verbose" = 'y' ]; then
    scripts/judge "$conf_file" || r=$?
  else
    scripts/judge "$conf_file" -n || r=$?
  fi
  if [ "$r" -ne 0 ]; then
    fatal "$prefix judge failed on $test_case"
    exit $r
  else
    green "$prefix judge passed on $test_case"
  fi
done
