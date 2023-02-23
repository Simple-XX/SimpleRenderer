
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
find_package(SDL2_ttf REQUIRED)
find_package(OpenMP REQUIRED)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${OpenMP_CXX_FLAGS}")

# Set include dir
include_directories(
        ${SimpleRender_SOURCE_DIR}/src/include
        ${SimpleRender_SOURCE_DIR}/src/3rd
        ${SDL2_INCLUDE_DIRS}
        ${SDL2_ttf_INCLUDE_DIRS}
        ${OpenMP_CXX_INCLUDE_DIRS})

set(test_src
        vector_test.cpp
        draw3d_test.cpp
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
        gtest_main
        ${glog_LIBRARIES}
        ${SDL2_LIBRARIES}
        # @todo 这里变量没有值，临时写一个
        -L/opt/homebrew/Cellar/sdl2_ttf/2.20.1/lib
        -L/opt/homebrew/lib -lSDL2_ttf -lSDL2
        ${OpenMP_CXX_LIBRARIES})

include(GoogleTest)
gtest_discover_tests(test)
