#!/bin/bash
set -e

docker run --platform linux/amd64 --rm -v $HOME/ydah/ycc:/ycc -w /ycc ycc "$@"
