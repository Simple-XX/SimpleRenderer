
# This file is a part of Simple-XX/SimpleRenderer
# (https://github.com/Simple-XX/SimpleRenderer).
#
# CMakeLists.txt for Simple-XX/SimpleRenderer.

# 设置项目名与版本
project(
        unit-test
        VERSION 0.0.1
)

enable_language(CXX)

add_executable(unit_test
        buffer_base_test.cpp
        #        render_test.cpp
        #        ${SimpleRender_SOURCE_DIR}/src/model.cpp
        #        ${SimpleRender_SOURCE_DIR}/src/display.cpp
        #        ${SimpleRender_SOURCE_DIR}/src/framebuffer.cpp
        #        ${SimpleRender_SOURCE_DIR}/src/camera.cpp
        #        ${SimpleRender_SOURCE_DIR}/src/shader.cpp
        #        ${SimpleRender_SOURCE_DIR}/src/default_shader.cpp
        #        ${SimpleRender_SOURCE_DIR}/src/color.cpp
        #        ${SimpleRender_SOURCE_DIR}/src/scene.cpp
        #        ${SimpleRender_SOURCE_DIR}/src/light.cpp
        #        ${SimpleRender_SOURCE_DIR}/src/input.cpp
        #        ${SimpleRender_SOURCE_DIR}/src/render.cpp
)

target_include_directories(unit_test PRIVATE
        ${SimpleRender_SOURCE_DIR}/src/include
)

target_compile_options(unit_test PRIVATE
        ${DEFAULT_TEST_COMPILE_OPTIONS}
)

target_link_options(unit_test PRIVATE
        ${DEFAULT_TEST_LINK_OPTIONS}
)

target_link_libraries(unit_test PRIVATE
        ${DEFAULT_TEST_LINK_LIB}
)

gtest_discover_tests(unit_test)
