
# This file is a part of Simple-XX/SimpleRenderer
# (https://github.com/Simple-XX/SimpleRenderer).
#
# CMakeLists.txt for Simple-XX/SimpleRenderer.

# Set CXX flags for debug
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g -ggdb -Wall -Wextra")

# gtest
include(FetchContent)
FetchContent_Declare(
  googletest
  URL https://github.com/google/googletest/archive/609281088cfefc76f9d0ce82e1ff6c30cc3591e5.zip
)
# For Windows: Prevent overriding the parent project's compiler/linker settings
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(googletest)

# Set include dir
include_directories(
    ${SimpleRender_SOURCE_DIR}/src/include
    ${Gtkmm4_INCLUDE_DIRS}
    ${glog_INCLUDE_DIRS}
    ${gflag_INCLUDE_DIRS})

aux_source_directory(${CMAKE_CURRENT_SOURCE_DIR} test_src)

enable_testing()

# 生成可执行文件
add_executable(test
    ${test_src})

target_link_libraries(test
    gtest_main
    ${glog_LIBRARIES})

include(GoogleTest)
gtest_discover_tests(test)
