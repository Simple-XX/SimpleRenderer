
# This file is a part of Simple-XX/SimpleRenderer
# (https://github.com/Simple-XX/SimpleRenderer).
#
# CMakeLists.txt for Simple-XX/SimpleRenderer.


# Automatically gather all source files
file(GLOB_RECURSE SOURCES "*.cpp" "*.c")

# Create the library
add_library(${PROJECT_NAME} STATIC ${SOURCES})

target_include_directories(${PROJECT_NAME} PRIVATE
        $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/src/include>
        $<INSTALL_INTERFACE:include>
)

target_compile_options(${PROJECT_NAME} PRIVATE
        ${DEFAULT_COMPILE_OPTIONS}
)

target_link_libraries(${PROJECT_NAME} PRIVATE
        ${DEFAULT_LINK_LIB}
)


target_link_libraries(${PROJECT_NAME} PRIVATE
        glad OpenGL::GL
)
