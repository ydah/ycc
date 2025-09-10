#!/bin/bash
set -e

docker buildx build --platform linux/amd64 -t ycc . --load
