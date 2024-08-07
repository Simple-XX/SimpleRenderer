
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
    ${SimpleRenderer_SOURCE_DIR}/include
)

list(APPEND DEFAULT_TEST_COMPILE_OPTIONS
    ${DEFAULT_COMPILE_OPTIONS}
    --coverage
)

list(APPEND DEFAULT_TEST_LINK_OPTIONS
    --coverage
    $<$<CXX_COMPILER_ID:GNU>:
    # -fsanitize=leak
    >
    # -fsanitize=address
    -fno-omit-frame-pointer
)

link_libraries(
    ${DEFAULT_LINK_LIB}
    gtest_main
    ${glog_LIBRARIES}
    SimpleRenderer
)

# Check if the platform is macOS
if(APPLE)
    # Enable RPATH support on macOS
    set(CMAKE_MACOSX_RPATH 1)

    # Set the RPATH to look for frameworks relative to the executable path
    set(CMAKE_INSTALL_RPATH "/Library/Frameworks")

    # Ensure that RPATH is used in the build
    set(CMAKE_BUILD_WITH_INSTALL_RPATH TRUE)
endif()


# add_subdirectory(unit_test)
#add_subdirectory(integration_test)
add_subdirectory(system_test)

# add_coverage_target(
#         DEPENDS unit_test
#         SOURCE_DIR ${SimpleRenderer_SOURCE_DIR}
#         BINARY_DIR ${SimpleRenderer_BINARY_DIR}
#         EXCLUDE_DIR ${SimpleRenderer_SOURCE_DIR}/3rd/*
# )
