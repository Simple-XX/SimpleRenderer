
# This file is a part of Simple-XX/SimpleRenderer
# (https://github.com/Simple-XX/SimpleRenderer).
#
# CMakeLists.txt for Simple-XX/SimpleRenderer.

# 生成静态库
add_library(${PROJECT_NAME} STATIC
        log_system.cpp
        color.cpp
        include/vector.hpp
        include/matrix.hpp
        include/exception.hpp
        include/model.hpp
        include/shader.h
        include/default_shader.h
        # include/light.h
        model.cpp
        light.cpp
        # scene.cpp
        shader.cpp
        # framebuffer.cpp
        default_shader.cpp
        simple_renderer.cpp
)

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
