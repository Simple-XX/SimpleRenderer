
# This file is a part of Simple-XX/SimpleRenderer (https://github.com/Simple-XX/SimpleRenderer).
#
# CMakeLists.txt for Simple-XX/SimpleRenderer.

# Set CXX flags for debug
if (CMAKE_BUILD_TYPE STREQUAL DEBUG)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g -ggdb -Wall -Wextra")
endif ()

if (CMAKE_BUILD_TYPE STREQUAL RELEASE)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra -Werror")
endif ()

# Set common flags
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O0")

message(STATUS "CMAKE_CXX_FLAGS is ${CMAKE_CXX_FLAGS}")

# Set include dir
include_directories(${SimpleRenderer_SOURCE_CODE_DIR}/include)

aux_source_directory(${SimpleRenderer_SOURCE_CODE_DIR} cxx_src)

# Link
add_executable(${RendererName}
    ${cxx_src})
