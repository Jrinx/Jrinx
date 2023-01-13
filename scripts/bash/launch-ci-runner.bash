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

RUNNER_ID="$1"
CONTAINER_NAME="jrinx-ci-$RUNNER_ID"
DEFAULT_IMAGE="ubuntu:22.04"

echo "Create $CONTAINER_NAME"

docker run -d \
  --name "$CONTAINER_NAME" \
  --restart always \
  -v /var/run/docker.sock:/var/run/docker.sock \
  gitlab/gitlab-runner:latest

echo "Register $CONTAINER_NAME as runner"

docker exec "$CONTAINER_NAME" gitlab-runner register \
  --non-interactive \
  --executor "docker" \
  --docker-image "$DEFAULT_IMAGE" \
  --url "$GITLAB_URL" \
  --registration-token "$REGISTRATION_TOKEN" \
  --description "ci runner for jrinx (No.$RUNNER_ID)" \
  --tag-list "docker" \
  --run-untagged="false" \
  --locked="false"
