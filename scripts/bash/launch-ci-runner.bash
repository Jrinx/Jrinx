#!/bin/bash

set -e

if [[ $# -ne 1 ]]; then
  exit 1
fi

if [[ ! -v GITLAB_URL ]]; then
  exit 2
fi

if [[ ! -v REGISTRATION_TOKEN ]]; then
  exit 3
fi

CONCURRENT="$1"

CONTAINER_NAME="jrinx-ci-runner"
DEFAULT_IMAGE="ubuntu:22.04"

echo "Create $CONTAINER_NAME"

docker run -d \
  --name "$CONTAINER_NAME" \
  --restart always \
  -v /var/run/docker.sock:/var/run/docker.sock \
  gitlab/gitlab-runner:latest

docker exec "$CONTAINER_NAME" sed -i \
  "s/concurrent.*/concurrent = $CONCURRENT/" \
  /etc/gitlab-runner/config.toml

docker exec "$CONTAINER_NAME" gitlab-runner register \
  --non-interactive \
  --executor "docker" \
  --docker-image "$DEFAULT_IMAGE" \
  --docker-cpus "$(nproc)" \
  --docker-cpuset-cpus "0-$(($(nproc)-1))"\
  --url "$GITLAB_URL" \
  --registration-token "$REGISTRATION_TOKEN" \
  --description "ci runner for jrinx (created at $(date))" \
  --tag-list "docker" \
  --run-untagged="false" \
  --locked="false"

echo "Register runner"

docker restart "$CONTAINER_NAME"
