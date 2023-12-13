
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
        matrix_test.cpp
)

target_compile_options(unit_test PRIVATE
        ${DEFAULT_TEST_COMPILE_OPTIONS}
)

target_link_options(unit_test PRIVATE
        ${DEFAULT_TEST_LINK_OPTIONS}
)

gtest_discover_tests(unit_test)

add_coverage_target(
        DEPENDS unit_test
        SOURCE_DIR ${SimpleRenderer_SOURCE_DIR}
        BINARY_DIR ${SimpleRenderer_BINARY_DIR}
        EXCLUDE_DIR ${SimpleRenderer_SOURCE_DIR}/3rd/*
)
