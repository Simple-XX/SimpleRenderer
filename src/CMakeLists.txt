
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

# opengl
find_package(OpenGL REQUIRED)

# glfw
find_package(glfw3 REQUIRED)
message(INFO ${glfw_INCLUDE_DIRS})
# Set include dir
include_directories(
        ./include
        ${OPENGL_INCLUDE_DIR}
        ${glfw_INCLUDE_DIRS})

aux_source_directory(${CMAKE_CURRENT_SOURCE_DIR} SimpleRender_src)

# 生成可执行文件
add_executable(SimpleRender
    ${SimpleRender_src})

target_link_libraries(SimpleRender
        ${OPENGL_LIBRARIES}
        ${glfw_LIBRARIES})
