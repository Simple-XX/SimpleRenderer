
# This file is a part of Simple-XX/SimpleRenderer
# (https://github.com/Simple-XX/SimpleRenderer).
#
# CMakeLists.txt for Simple-XX/SimpleRenderer.

# 设置最小 cmake 版本
cmake_minimum_required(VERSION 3.27 FATAL_ERROR)

# 设置C++标准
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# 设置项目名与版本
project(SimpleRenderer
        VERSION 0.0.1)

# 禁止原地编译
if (${PROJECT_SOURCE_DIR} STREQUAL ${PROJECT_BINARY_DIR})
    # 如果你看到这句话，cmake 此时已经在根目录下生成了一些临时文件，你需要删除它们
    # CMakeFiles, CMakeCache.txt
    message(
            FATAL_ERROR
            "In-source builds not allowed. Please make a new directory (called a build directory) and run CMake from there."
    )
endif ()

# 设置辅助 cmake 脚本路径
list(APPEND CMAKE_MODULE_PATH "${PROJECT_SOURCE_DIR}/cmake")

# 导入函数
include(functions)
# 导入第三方依赖
include(3rd)
# 导入编译配置
include(compile_config)

# obj 文件路径
set(OBJ_FILE_PATH "${PROJECT_SOURCE_DIR}/obj/")
# 字体文件路径
set(FONT_FILE_PATH "${wqy_font_SOURCE_DIR}/wqy-zenhei.ttc")
# 线程数
include(ProcessorCount)
ProcessorCount(NPROC)
# 日志文件路径
set(LOG_FILE_PATH "${EXECUTABLE_OUTPUT_PATH}/logs/SimpleRendererLog.log")
# 日志文件大小
set(LOG_FILE_MAX_SIZE 1024*1024*4)
# 日志文件数量
set(LOG_FILE_MAX_COUNT 8)
# 生成配置头文件
configure_file(
        "${PROJECT_SOURCE_DIR}/cmake/config.h.in"
        "${PROJECT_SOURCE_DIR}/src/include/config.h"
)

# 添加要编译的目录
add_subdirectory(src)
add_subdirectory(test)
add_subdirectory(doc)
