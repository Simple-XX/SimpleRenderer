# This file is a part of Simple-XX/SimpleRenderer (https://github.com/Simple-XX/SimpleRenderer).
# compile_config.cmake for Simple-XX/SimpleRenderer. Compile configurations.

# Compile options
list(APPEND DEFAULT_COMPILE_OPTIONS
    -Wall
    -Wextra
    $<$<CONFIG:Release>:-O3;-Werror>
    $<$<CONFIG:Debug>:-O0;-g;-ggdb>
)

# Default libraries to link
list(APPEND DEFAULT_LINK_LIB
    spdlog::spdlog
    stb
    tinyobjloader
    Eigen
    ${glog_LIBRARIES}
    OpenGL::GL
    glad
    raylib
    OpenMP::OpenMP_CXX
)
