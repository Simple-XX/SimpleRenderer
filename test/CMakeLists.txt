
# This file is a part of Simple-XX/SimpleRenderer
# (https://github.com/Simple-XX/SimpleRenderer).
#
# CMakeLists.txt for Simple-XX/SimpleRenderer.

# 设置项目名与版本
project(
        test
        VERSION 0.0.1
)

enable_testing()
include(GoogleTest)

include_directories(
        ${SimpleRenderer_SOURCE_DIR}/src/include
)

list(APPEND DEFAULT_TEST_COMPILE_OPTIONS
        ${DEFAULT_COMPILE_OPTIONS}
        --coverage
)

list(APPEND DEFAULT_TEST_LINK_OPTIONS
        --coverage
        $<$<CXX_COMPILER_ID:GNU>:
        -fsanitize=leak
        >
        -fsanitize=address
        -fno-omit-frame-pointer
)

link_libraries(
        ${DEFAULT_LINK_LIB}
        gtest_main
        ${glog_LIBRARIES}
        SimpleRenderer-shared
)

add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/unit_test)
#add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/integration_test)
add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/system_test)

add_coverage_target(
        DEPENDS unit_test
        SOURCE_DIR ${CMAKE_SOURCE_DIR}
        BINARY_DIR ${CMAKE_BINARY_DIR}
        EXCLUDE_DIR ${CMAKE_SOURCE_DIR}/3rd/*
)
