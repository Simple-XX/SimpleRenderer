
# This file is a part of Simple-XX/SimpleRenderer
# (https://github.com/Simple-XX/SimpleRenderer).
#
# CMakeLists.txt for Simple-XX/SimpleRenderer.

# Set CXX flags for debug
if (CMAKE_BUILD_TYPE STREQUAL DEBUG)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g -ggdb -Wall -Wextra")
elseif (CMAKE_BUILD_TYPE STREQUAL RELEASE)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra -Werror")
else ()
    set(CMAKE_BUILD_TYPE DEBUG)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g -ggdb -Wall -Wextra")
endif ()

# Set common flags
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O0")

message(STATUS "CMAKE_CXX_FLAGS is ${CMAKE_CXX_FLAGS}")

# Set include dir
include_directories(
        ./include
        ./3rd)

set(SimpleRender_src
        main.cpp
        model.cpp
        framebuffer.cpp
        display.cpp
        camera.cpp
        shader.cpp
        default_shader.cpp
        color.cpp
        scene.cpp
        light.cpp
        input.cpp
        render.cpp)

# 生成可执行文件
add_executable(SimpleRender
        ${SimpleRender_src})

target_link_libraries(SimpleRender
        SDL2::Main
        SDL2::TTF
        OpenMP::OpenMP_CXX)
