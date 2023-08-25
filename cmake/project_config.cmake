
# This file is a part of Simple-XX/SimpleRenderer
# (https://github.com/Simple-XX/SimpleRenderer).
#
# project_config.cmake for Simple-XX/SimpleRenderer.
# 配置信息

# obj 文件路径
set(OBJ_FILE_PATH "${PROJECT_SOURCE_DIR}/obj/")
# 字体文件路径
set(FONT_FILE_PATH "${PROJECT_SOURCE_DIR}/3rd/wqy-zenhei/")
# 线程数
include(ProcessorCount)
ProcessorCount(NPROC)

# 生成配置头文件
configure_file(
        "${PROJECT_SOURCE_DIR}/cmake/config.h.in"
        "${PROJECT_SOURCE_DIR}/src/include/config.h"
)
