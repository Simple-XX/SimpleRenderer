
# This file is a part of Simple-XX/SimpleRenderer
# (https://github.com/Simple-XX/SimpleRenderer).
#
# CMakeLists.txt for Simple-XX/SimpleRenderer.

# 生成静态库
file(GLOB_RECURSE SRC_FILES CONFIGURE_DEPENDS 
    "*.cpp"
    "*.c"
)
add_library(${PROJECT_NAME} STATIC ${SRC_FILES})

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
