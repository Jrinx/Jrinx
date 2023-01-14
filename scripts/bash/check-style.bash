#!/bin/bash

set -e

# editor-config checker
[ -z "$EC" ] && EC=ec

# shellscript checker
[ -z "$SHELLCHECK" ] && SHELLCHECK=shellcheck

# clang-format
[ -z "$CLANG_FORMAT" ] && CLANG_FORMAT=clang-format

# autopep8
[ -z "$AUTOPEP8" ] && AUTOPEP8=autopep8

fix="no"
opt="lescp"
dry="no"
line_len_max=96

while [[ $# -gt 0 ]]
do
  case $1 in
  -f|--fix)
    fix="yes"
    shift
    ;;
  -c|--check)
    if [ $# -lt 2 ]; then
      echo "Expect spec check following -c/--check (choose from 'escp')"
      echo "  l  line-length-max check"
      echo "  e  editorconfig check"
      echo "  s  shellscript check"
      echo "  c  clang-format check/format"
      echo "  p  python-pep8 check/format"
      exit 1
    fi
    shift
    opt="$1"
    if [[ "$opt" =~ [^lescp] ]]; then
      echo "Unknown spec check $opt"
      exit 1
    fi
    shift
    ;;
  -d|--dry-run)
    dry="yes"
    shift
    ;;
  *)
    echo "Unknown option $1"
    exit 1
    ;;
  esac
done

source "${0%/*}"/bash/utils.bash

all_files=$(
  echo -e "$(git ls-files --full-name)" | \
  while IFS= read -r f; do \
    [[ -f "$f" ]] && echo "$f"; \
  done | \
  grep -ve '\.md$' | \
  grep -Fxv "$(grep '^\s*\[submodule ' .gitmodules | cut -d '"' -f2)"
)

function check-ln() {
  if [ "$dry" = 'yes' ]; then
    green '[ line-length-max checker ] just show files:'
  else
    green '[ line-length-max checker ] begin'
  fi

  line_len_max_check="$(cat <<-EOF
BEGIN {
  cnt = 0
}
{
  if (length > $line_len_max) {
    print FILENAME ":" NR;
    cnt = cnt + 1;
  }
}
END {
  exit cnt
}
EOF
)"

  if [ "$dry" = 'yes' ]; then
    echo "$all_files"
  elif ! echo "$all_files" | xargs -n1 gawk "$line_len_max_check" ; then
    fatal '[ line-length-max checker ] failed'
    return 1
  else
    green '[ line-length-max checker ] passed'
  fi
  return 0
}

function check-ec() {
  if [ "$dry" = 'yes' ]; then
    green '[ editor-config checker ] just show files'
  else
    green '[ editor-config checker ] begin'
  fi

  if [ "$dry" = 'yes' ]; then
    $EC -dry-run
  elif ! $EC ; then
    fatal '[ editor-config checker ] failed'
    return 1
  else
    green '[ editor-config checker ] passed'
  fi
  return 0
}

function check-sh() {
  if [ "$dry" = 'yes' ]; then
    green '[ shellcheck ] just show bash files:'
  else
    green '[ shellcheck ] begin'
  fi

  bash_files=$(echo "$all_files" | grep '[a-zA-Z_-]\+\.bash$')
  if [ -z "$bash_files" ]; then
    green '[ shellcheck ] no bash files found'
  elif [ "$dry" = 'yes' ]; then
    echo "$bash_files"
  elif # shellcheck disable=2086
    ! $SHELLCHECK -x $bash_files -e 1090,1091 ; then
    fatal '[ shellcheck ] failed'
    return 1
  else
    green '[ shellcheck ] passed'
  fi
  return 0
}

function check-cl() {
  if [ "$dry" = 'yes' ]; then
    green '[ clang-format ] just show c files:'
  else
    green '[ clang-format ] begin'
  fi

  c_files=$(echo "$all_files" | grep '[a-zA-Z_-]\+\.\(c\|h\)$')
  if [  -z "$c_files" ]; then
    green '[ clang-format ] no c files found'
  elif [ "$dry" = 'yes' ]; then
    echo "$c_files"
  elif [ "$fix" = 'yes' ]; then
    # shellcheck disable=2086
    $CLANG_FORMAT -i $c_files
    green '[ clang-format ] fix end'
  else
    # shellcheck disable=2086
    cnt=$($CLANG_FORMAT $c_files --output-replacements-xml | grep -c "<replacement ")
    if [ "$cnt" -eq 0 ]; then
      green '[ clang-format ] check passed'
    else
      fatal "[ clang-format ] check failed"
      return 1
    fi
  fi
  return 0
}

function check-py() {
  if [ "$dry" = 'yes' ]; then
    green '[ autopep8 ] just show py files:'
  else
    green '[ autopep8 ] begin'
  fi

  py_files=$(echo "$all_files" | grep '[a-zA-Z_-]\+\.py$')
  if [ -z "$py_files" ]; then
    green '[ autopep8 ] no python files found'
  elif [ "$dry" = 'yes' ]; then
    echo "$py_files"
  elif [ "$fix" = 'yes' ]; then
    # shellcheck disable=2086
    $AUTOPEP8 --max-line-length $line_len_max -i -j 0 $py_files
    green '[ autopep8 ] fix end'
  else
    # shellcheck disable=2086
    if ! $AUTOPEP8 --max-line-length $line_len_max --exit-code -d $py_files ; then
      fatal '[ autopep8 ] check failed'
      return 1
    else
      green '[ autopep8 ] check passed'
    fi
  fi
  return 0
}

r=0
if [[ "$opt" == *"p"* ]] && ! check-py; then
  r=$((r+2))
fi
if [[ "$opt" == *"c"* ]] && ! check-cl; then
  r=$((r+4))
fi
if [[ "$opt" == *"s"* ]] && ! check-sh; then
  r=$((r+8))
fi
if [[ "$opt" == *"e"* ]] && ! check-ec; then
  r=$((r+16))
fi
if [[ "$opt" == *"l"* ]] && ! check-ln; then
  r=$((r+32))
fi
exit $r
