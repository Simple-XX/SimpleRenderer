
# This file is a part of Simple-XX/SimpleRenderer
# (https://github.com/Simple-XX/SimpleRenderer).
#
# compile_config.cmake for Simple-XX/SimpleRenderer.
# 配置信息

# 编译选项
list(APPEND DEFAULT_COMPILE_OPTIONS
        -Wall
        -Wextra
        $<$<CONFIG:Release>:-O3;-Werror>
        $<$<CONFIG:Debug>:-O0;-g;-ggdb>
)

list(APPEND DEFAULT_LINK_LIB
        spdlog::spdlog
        stb
        glm::glm
        ${glog_LIBRARIES}
        SDL2::SDL2
        OpenMP::OpenMP_CXX
        assimp
)
