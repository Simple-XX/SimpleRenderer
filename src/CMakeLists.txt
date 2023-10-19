
# This file is a part of Simple-XX/SimpleRenderer
# (https://github.com/Simple-XX/SimpleRenderer).
#
# CMakeLists.txt for Simple-XX/SimpleRenderer.

# 生成库需要的源文件
set(lib_src
        log.cpp
        color.cpp
        model.cpp
        camera.cpp
        light.cpp
        scene.cpp
        shader.cpp
        framebuffer.cpp
        display.cpp
        default_shader.cpp
        input.cpp
        render.cpp
)

# 生成动态库
add_library(${PROJECT_NAME}-shared SHARED
        ${lib_src}
)

target_include_directories(
        ${PROJECT_NAME}-shared PRIVATE $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/src/include>
        $<INSTALL_INTERFACE:include/${PROJECT_NAME}-${PROJECT_VERSION}>
)

target_compile_options(${PROJECT_NAME}-shared PRIVATE
        ${DEFAULT_COMPILE_OPTIONS}
)

target_link_libraries(${PROJECT_NAME}-shared PRIVATE
        ${DEFAULT_LINK_LIB}
)

# 生成静态库
add_library(${PROJECT_NAME}-static STATIC
        ${lib_src}
)

target_include_directories(
        ${PROJECT_NAME}-static PRIVATE $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/src/include>
        $<INSTALL_INTERFACE:include/${PROJECT_NAME}-${PROJECT_VERSION}>
)

target_compile_options(${PROJECT_NAME}-static PRIVATE
        ${DEFAULT_COMPILE_OPTIONS}
)

target_link_libraries(${PROJECT_NAME}-static PRIVATE
        ${DEFAULT_LINK_LIB}
)

# 打包
# @todo 导出静态库与动态库
packageProject(
        # 要导出的 target
        NAME ${PROJECT_NAME}-shared
        # 版本
        VERSION ${PROJECT_VERSION}
        # 生成文件目录
        BINARY_DIR ${PROJECT_BINARY_DIR}
        # 头文件路径
        INCLUDE_DIR ${PROJECT_SOURCE_DIR}/src/include
        # 与 target 的 INSTALL_INTERFACE 一致
        INCLUDE_DESTINATION include/${PROJECT_NAME}-${PROJECT_VERSION}
        # 头文件过滤
        INCLUDE_HEADER_PATTERN "*.h"
        # namespace 名称，不能包含 '::'
        NAMESPACE ${PROJECT_NAMESPACE}
        # (optional) option to disable the versioning of install destinations
        DISABLE_VERSION_SUFFIX YES
        ARCH_INDEPENDENT YES
        # 生成 CPack 变量
        CPACK YES
)
