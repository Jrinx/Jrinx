#!/bin/bash

function green() {
  echo -e "\033[0;32m$1\033[0m"
}

function bgreen() {
  echo -e "\033[32m\033[01m$1\033[0m"
}

function warning() {
  echo -e "\033[0;33m$1\033[0m"
}

function fatal() {
  echo -e "\033[0;31m$1\033[0m"
}

function bfatal() {
  echo -e "\033[31m\033[01m$1\033[0m"
}
