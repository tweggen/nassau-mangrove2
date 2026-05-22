cmake_minimum_required(VERSION 3.15)
project(MangroveCompressor)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra -Wpedantic")

# DSP Library
add_subdirectory(Source/DSP)

# Tests
enable_testing()
add_subdirectory(Tests)
