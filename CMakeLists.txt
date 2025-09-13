cmake_minimum_required(VERSION 3.10)
project(gpu_stats_test)

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Add AMD SMI library
include_directories(${CMAKE_SOURCE_DIR}/amdsmi/include)
link_directories(${CMAKE_SOURCE_DIR}/amdsmi/build/src)

add_executable(gpu_stats_test main.cpp)
target_link_libraries(gpu_stats_test PRIVATE amd_smi)
