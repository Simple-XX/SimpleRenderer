
# This file is a part of Simple-XX/SimpleRenderer
# (https://github.com/Simple-XX/SimpleRenderer).
#
# CMakeLists.txt for Simple-XX/SimpleRenderer.

# Set CXX flags for debug
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra")

# Set include dir
include_directories(
        ${SimpleRender_SOURCE_DIR}/src/include
)

set(test_src
        vector_test.cpp
        render_test.cpp
        matrix_test.cpp
        color_test.cpp
        ${SimpleRender_SOURCE_DIR}/src/model.cpp
        ${SimpleRender_SOURCE_DIR}/src/display.cpp
        ${SimpleRender_SOURCE_DIR}/src/framebuffer.cpp
        ${SimpleRender_SOURCE_DIR}/src/camera.cpp
        ${SimpleRender_SOURCE_DIR}/src/shader.cpp
        ${SimpleRender_SOURCE_DIR}/src/default_shader.cpp
        ${SimpleRender_SOURCE_DIR}/src/color.cpp
        ${SimpleRender_SOURCE_DIR}/src/scene.cpp
        ${SimpleRender_SOURCE_DIR}/src/light.cpp
        ${SimpleRender_SOURCE_DIR}/src/input.cpp
        ${SimpleRender_SOURCE_DIR}/src/render.cpp)

enable_testing()

# 生成可执行文件
add_executable(test
        ${test_src})

target_link_libraries(test
        stb
        tinyobjloader
        Eigen
        gtest_main
        ${glog_LIBRARIES}
        SDL2::Main
        SDL2::TTF
        OpenMP::OpenMP_CXX)

include(GoogleTest)
gtest_discover_tests(test)
