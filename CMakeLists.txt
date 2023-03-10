cmake_minimum_required(VERSION 3.9)

# The compiler to use.
set(CMAKE_CXX_COMPILER "g++")
# Get the best optimizations available.
set(CMAKE_CXX_FLAGS "--std=c++17 -O3 -march=native -mtune=native -fopenmp")

# Specify default build type to "Release".
set(CMAKE_BUILD_TYPE Release)

# The name of the executable.
project(knng)

find_package(OpenMP REQUIRED)

# Find all the source and header files.
FILE(GLOB SRCS src/*)
FILE(GLOB HDRS include/*)

# Specify the project's files, dependencies and libraries.
add_executable(${PROJECT_NAME} ${SRCS} ${HDRS})
target_include_directories(${PROJECT_NAME} PRIVATE include)
