
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

find_package(SDL2 REQUIRED)
find_package(SDL2_ttf REQUIRED)

# Set include dir
include_directories(
        ./include
        ./3rd
        ${SDL2_INCLUDE_DIRS}
        ${SDL2_ttf_INCLUDE_DIRS})

set(SimpleRender_src
        main.cpp
        model.cpp
        framebuffer.cpp
        display.cpp
        draw3d.cpp
        camera.cpp
        shader.cpp
        default_shader.cpp
        event_callback.cpp
        color.cpp)

# 生成可执行文件
add_executable(SimpleRender
        ${SimpleRender_src})

target_link_libraries(SimpleRender
        ${SDL2_LIBRARIES}
        ${SDL2_ttf_LIBRARIES}
        # @todo 这里变量没有值，临时写一个
        -L/opt/homebrew/Cellar/sdl2_ttf/2.20.1/lib
        -L/opt/homebrew/lib -lSDL2_ttf -lSDL2)
