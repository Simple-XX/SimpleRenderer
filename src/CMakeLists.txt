
# This file is a part of Simple-XX/SimpleRenderer
# (https://github.com/Simple-XX/SimpleRenderer).
#
# CMakeLists.txt for Simple-XX/SimpleRenderer.

# 生成可执行文件
add_executable(SimpleRender
        main.cpp
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

target_include_directories(SimpleRender PRIVATE
        include
)

target_compile_options(SimpleRender PRIVATE
        ${DEFAULT_COMPILE_OPTIONS}
)

target_link_libraries(SimpleRender PRIVATE
        ${DEFAULT_LINK_LIB}
)
