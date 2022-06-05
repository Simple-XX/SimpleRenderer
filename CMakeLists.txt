
# This file is a part of Simple-XX/SimpleRenderer
# (https://github.com/Simple-XX/SimpleRenderer).
#
# CMakeLists.txt for Simple-XX/SimpleRenderer.

# Set minimum cmake version
cmake_minimum_required(VERSION 3.10)

project(SimpleRender LANGUAGES CXX)

if(${SimpleRenderer_SOURCE_DIR} STREQUAL ${SimpleRenderer_BINARY_DIR})
    message(FATAL_ERROR "In-source builds not allowed. Please make a new directory (called a build directory) and run CMake from there.")
endif()

# Set debug
set(CMAKE_BUILD_TYPE DEBUG)

# Set C gnu17
set(CMAKE_CXX_STANDARD 17)

set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

# Set renderer name
set(RendererName renderer)

# pkgconfig
find_package(PkgConfig REQUIRED)

# gtk
pkg_check_modules(Gtkmm4 REQUIRED gtkmm-4.0)
add_definitions(${Gtkmm4_CFLAGS_OTHER})
link_directories(${Gtkmm4_LIBRARY_DIRS})

# gflag
pkg_check_modules(gflag REQUIRED gflags)
add_definitions(${gflag_CFLAGS_OTHER})
link_directories(${gflag_LIBRARY_DIRS})

# glog
pkg_check_modules(glog REQUIRED libglog)
add_definitions(${glog_CFLAGS_OTHER})
link_directories(${glog_LIBRARY_DIRS})

add_subdirectory(${SimpleRender_SOURCE_DIR}/src)
# add_subdirectory(${SimpleRender_SOURCE_DIR}/test)
