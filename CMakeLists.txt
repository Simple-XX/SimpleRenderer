
# This file is a part of Simple-XX/SimpleRenderer (https://github.com/Simple-XX/SimpleRenderer).
#
# CMakeLists.txt for Simple-XX/SimpleRenderer.

# Set minimum cmake version
cmake_minimum_required(VERSION 3.10)

project(SimpleRenderer LANGUAGES CXX)

if(${SimpleRenderer_SOURCE_DIR} STREQUAL ${SimpleRenderer_BINARY_DIR})
    message(FATAL_ERROR "In-source builds not allowed. Please make a new directory (called a build directory) and run CMake from there.")
endif()

# Set debug
set(CMAKE_BUILD_TYPE DEBUG)

# Set C gnu11
set(CMAKE_CXX_STANDARD 11)

set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

# Set kernel name
set(RendererName renderer)

set(SimpleRenderer_SOURCE_CODE_DIR ${SimpleRenderer_SOURCE_DIR}/src)
add_subdirectory(${SimpleRenderer_SOURCE_CODE_DIR})
