#!/bin/bash
PWD=$(pwd)

# Build gpu-stats-test
mkdir -p build && cd build && cmake .. && make -j$(nproc)
