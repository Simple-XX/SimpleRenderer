
# This file is a part of Simple-XX/SimpleRenderer
# (https://github.com/Simple-XX/SimpleRenderer).
#
# CMakeLists.txt for Simple-XX/SimpleRenderer.

# Set CXX flags for debug
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra")

# gtest
include(FetchContent)
FetchContent_Declare(
        googletest
        GIT_REPOSITORY https://github.com/google/googletest.git
        GIT_TAG release-1.12.1
)
# For Windows: Prevent overriding the parent project's compiler/linker settings
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(googletest)

find_package(SDL2 REQUIRED)

# Set include dir
include_directories(
        ${SimpleRender_SOURCE_DIR}/src/include
        ${SDL2_INCLUDE_DIRS})

set(test_src
        vector_test.cpp
        draw3d_test.cpp
        matrix_test.cpp
        ../src/display.cpp
        ../src/framebuffer.cpp
        ../src/camera.cpp)

enable_testing()

# 生成可执行文件
add_executable(test
        ${test_src})

target_link_libraries(test
        gtest_main
        ${glog_LIBRARIES}
        ${SDL2_LIBRARIES})

include(GoogleTest)
gtest_discover_tests(test)
