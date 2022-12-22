#!/usr/bin/env bash

function green() {
  echo -e "\033[0;32m$1\033[0m"
}

function warning() {
  echo -e "\033[0;33m$1\033[0m"
}

function fatal() {
  echo -e "\033[0;31m$1\033[0m"
}
