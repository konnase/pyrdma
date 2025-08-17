#!/bin/bash

# Script to build the project using CMake
BASE_DIR=$(dirname "$0")/..
cd $BASE_DIR

rm -rf build
mkdir -p build
cd build
cmake ..
make
