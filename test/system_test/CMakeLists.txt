# This file is a part of Simple-XX/SimpleRenderer (https://github.com/Simple-XX/SimpleRenderer).
# CMakeLists.txt for Simple-XX/SimpleRenderer system tests.

# Set project name and version
project(system_test VERSION 0.0.1)

# Enable C++ language
enable_language(CXX)

# Collect all source files
file(GLOB SOURCES "${CMAKE_CURRENT_SOURCE_DIR}/*.cpp")

# Create the executable
add_executable(system_test ${SOURCES})

# Include directories
target_include_directories(system_test PRIVATE
    ${CMAKE_SOURCE_DIR}/src/include
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${raylib_INCLUDE_DIRS}
)

# Compile options
target_compile_options(system_test PRIVATE ${DEFAULT_TEST_COMPILE_OPTIONS})

# Link options
target_link_options(system_test PRIVATE ${DEFAULT_TEST_LINK_OPTIONS})

# Link libraries
target_link_libraries(system_test PRIVATE
    glad
    raylib
    OpenGL::GL
    ${DEFAULT_LINK_LIB}
)

# Apple-specific frameworks
if (APPLE)
    target_link_libraries(system_test PRIVATE "-framework IOKit" "-framework Cocoa" "-framework OpenGL")
endif ()
