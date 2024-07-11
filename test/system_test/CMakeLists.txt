
# This file is a part of Simple-XX/SimpleRenderer
# (https://github.com/Simple-XX/SimpleRenderer).
#
# CMakeLists.txt for Simple-XX/SimpleRenderer.

# 设置项目名与版本
project(
        system-test
        VERSION 0.0.1
)

enable_language(CXX)

file(GLOB SOURCES "${CMAKE_SOURCE_DIR}/test/system_test/*.cpp")

add_executable(main
        ${SOURCES}
)

target_include_directories(main PRIVATE
        ${CMAKE_SOURCE_DIR}/src/include
        ${CMAKE_SOURCE_DIR}/test/system_test
)

target_compile_options(main PRIVATE
        ${DEFAULT_TEST_COMPILE_OPTIONS}
)

target_link_options(main PRIVATE
        ${DEFAULT_TEST_LINK_OPTIONS}
)

target_link_libraries(main PRIVATE
        glad raylib OpenGL::GL
)

# if (APPLE)
#         target_link_libraries(system_test PRIVATE
#         ${PROJECT_NAME} "-framework IOKit")
#         target_link_libraries(system_test PRIVATE
#         ${PROJECT_NAME} "-framework Cocoa")
#         target_link_libraries(system_test PRIVATE
#         ${PROJECT_NAME} "-framework OpenGL")
# endif()