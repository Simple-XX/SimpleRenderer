
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
add_library(SimpleRenderer-shared SHARED
        ${lib_src}
)

target_include_directories(SimpleRenderer-shared PRIVATE
        include
)

target_compile_options(SimpleRenderer-shared PRIVATE
        ${DEFAULT_COMPILE_OPTIONS}
)

target_link_libraries(SimpleRenderer-shared PRIVATE
        ${DEFAULT_LINK_LIB}
)

# 生成静态库
add_library(SimpleRenderer-static STATIC
        ${lib_src}
)

target_include_directories(SimpleRenderer-static PRIVATE
        include
)

target_compile_options(SimpleRenderer-static PRIVATE
        ${DEFAULT_COMPILE_OPTIONS}
)

target_link_libraries(SimpleRenderer-static PRIVATE
        ${DEFAULT_LINK_LIB}
)

# 生成可执行文件
add_executable(SimpleRender
        main.cpp
)

target_include_directories(SimpleRender PRIVATE
        include
)

target_compile_options(SimpleRender PRIVATE
        ${DEFAULT_COMPILE_OPTIONS}
)

target_link_libraries(SimpleRender PRIVATE
        ${DEFAULT_LINK_LIB}
        SimpleRenderer-shared
)
