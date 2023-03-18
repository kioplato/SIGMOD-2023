cmake_minimum_required(VERSION 3.9)

# The compiler to use.
set(CMAKE_CXX_COMPILER "g++")
# Get the best optimizations available.
set(CMAKE_CXX_FLAGS "--std=c++17 -O3 -march=native -mtune=native -fopenmp")

# Specify default build type to "Release".
set(CMAKE_BUILD_TYPE Release)

# The name of the executable.
project(knng)

# Verify that OpenMP exists.
find_package(OpenMP REQUIRED)

if (OpenMP_CXX_not_FOUND)
	message("OpenMP is required but not found. Abort.")
endif()

# Compile project with VERBOSE macro defined for verbose execution.
if (VERBOSE)
	add_compile_definitions(VERBOSE)
endif()

# Find all the source and header files.
FILE(GLOB SRCS src/*)

# Specify the project's files, dependencies and libraries.
add_executable(${PROJECT_NAME} ${SRCS})
# Include the header files.
target_include_directories(${PROJECT_NAME} PRIVATE include)
# Link the OpenMP library.
target_link_libraries(${PROJECT_NAME} PUBLIC OpenMP::OpenMP_CXX)
