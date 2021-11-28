#!/bin/bash

# build ubuntu
mkdir -p deploy/release/ubuntu
docker build -t nanoinit-builder-ubuntu -f deploy/Dockerfile.ubuntu .
docker run --name nanoinit-builder-ubuntu -v $(pwd)/deploy/release/ubuntu:/opt/release/ nanoinit-builder-ubuntu
docker rm nanoinit-builder-ubuntu
docker rmi nanoinit-builder-ubuntu

#build alpine
mkdir -p deploy/release/alpine
docker build -t nanoinit-builder-alpine -f deploy/Dockerfile.alpine .
docker run --name nanoinit-builder-alpine -v $(pwd)/deploy/release/alpine:/opt/release/ nanoinit-builder-alpine
docker rm nanoinit-builder-alpine
docker rmi nanoinit-builder-alpine
