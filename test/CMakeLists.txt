# This file is a part of Simple-XX/SimpleRenderer (https://github.com/Simple-XX/SimpleRenderer).
# CMakeLists.txt for Simple-XX/SimpleRenderer tests.

# Set project name and version
project(test VERSION 0.0.1)

# Enable testing
enable_testing()
include(GoogleTest)

# Include directories
include_directories(
    ${CMAKE_SOURCE_DIR}/src/include
    ${CMAKE_SOURCE_DIR}/test/system_test
)

# Define compile options for tests
list(APPEND DEFAULT_TEST_COMPILE_OPTIONS
    ${DEFAULT_COMPILE_OPTIONS}
    --coverage
)

# Define link options for tests
list(APPEND DEFAULT_TEST_LINK_OPTIONS
    --coverage
    $<$<CXX_COMPILER_ID:GNU>:
        # -fsanitize=leak
    >
    # -fsanitize=address
    -fno-omit-frame-pointer
)

# Link libraries
link_libraries(
    ${DEFAULT_LINK_LIB}
    gtest_main
    ${glog_LIBRARIES}
    SimpleRenderer
    glad 
    raylib
    OpenGL::GL
)

# Add subdirectories for different test types
add_subdirectory(system_test)
# add_subdirectory(unit_test)
# add_subdirectory(integration_test)

# Optional coverage target
# add_coverage_target(
#     DEPENDS unit_test
#     SOURCE_DIR ${SimpleRenderer_SOURCE_DIR}
#     BINARY_DIR ${SimpleRenderer_BINARY_DIR}
#     EXCLUDE_DIR ${SimpleRenderer_SOURCE_DIR}/3rd/*
# )
