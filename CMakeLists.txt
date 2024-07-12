# This file is a part of Simple-XX/SimpleRenderer (https://github.com/Simple-XX/SimpleRenderer).
# CMakeLists.txt for Simple-XX/SimpleRenderer.

# Set the minimum CMake version
cmake_minimum_required(VERSION 3.27 FATAL_ERROR)

# Set the C++ standard
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Set project name and version
project(SimpleRenderer VERSION 0.0.1)

# Prevent in-source builds
if (${PROJECT_SOURCE_DIR} STREQUAL ${PROJECT_BINARY_DIR})
    message(FATAL_ERROR "In-source builds are not allowed. Please create a build directory and run CMake from there.")
endif()

# Set auxiliary CMake script path
list(APPEND CMAKE_MODULE_PATH "${PROJECT_SOURCE_DIR}/cmake")

# Include custom CMake functions
include(functions)

# Include third-party dependencies
include(3rd)

# Include compile configurations
include(compile_config)

# Define paths for resources
set(OBJ_FILE_PATH "${PROJECT_SOURCE_DIR}/obj/")
set(FONT_FILE_PATH "${wqy_font_SOURCE_DIR}/wqy-zenhei.ttc")

# Determine number of processor cores
include(ProcessorCount)
ProcessorCount(NPROC)

# Define logging configurations
set(LOG_FILE_PATH "${EXECUTABLE_OUTPUT_PATH}/logs/SimpleRendererLog.log")
set(LOG_FILE_MAX_SIZE 4194304)  # 4 MB
set(LOG_FILE_MAX_COUNT 8)

# Generate configuration header file
configure_file(
    "${PROJECT_SOURCE_DIR}/cmake/config.h.in"
    "${PROJECT_SOURCE_DIR}/src/include/config.h"
)

# Find and include OpenGL
find_package(OpenGL REQUIRED)
find_package(raylib REQUIRED)

# Add glad library
add_library(glad "${PROJECT_SOURCE_DIR}/src/glad.c")
target_include_directories(glad PUBLIC "${PROJECT_SOURCE_DIR}/src/include")

# Add project subdirectories
add_subdirectory(src)
add_subdirectory(test)
add_subdirectory(doc)
