#!/bin/bash
PWD=$(pwd)

# Build amdsmi
cd amdsmi && mkdir build && cd build && cmake .. && make -j$(nproc)
cd $PWD

# Build gpu-stats-test
mkdir -p build
cd build
cmake ..
make -j$(nproc)
