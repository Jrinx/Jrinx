#!/bin/bash

set -e

[ ! -v MODE ] && MODE=debug

debug='no'

while [[ $# -gt 0 ]]
do
  case $1 in
  -t|--test)
    if [ $# -lt 2 ]; then
      echo 'Expect test name following -t/--test'
      exit 1
    fi
    shift
    TEST="$1"
    shift
    ;;
  -d|--debug)
    debug='yes'
    shift
    ;;
  *)
    echo "Unknown option $1"
    exit 1
    ;;
  esac
done

source "${0%/*}"/bash/utils.bash

if [ -z "$TEST" ]; then
  fatal 'Expect test name as argument'
  exit 1
fi

if [ "$debug" = 'yes' ]; then
  test_out="$TEST.test-out"
  touch "$test_out"
else
  test_out="$(mktemp)"
fi

function runtest() {
  make $MODE COLOR=n TEST="$TEST"
  make run | tee "$test_out"
  if grep -q 'arg-driven test done, halt!' "$test_out"; then
    return 0
  else
    return 1
  fi
}

prefix="[ run-test ($MODE: $TEST) ]"

green "$prefix begin"
if runtest; then
  green "$prefix passed"
  exit 0
else
  fatal "$prefix failed"
  exit 1
fi
