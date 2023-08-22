
# This file is a part of Simple-XX/SimpleRenderer
# (https://github.com/Simple-XX/SimpleRenderer).
#
# functions.cmake for Simple-XX/SimpleRenderer.
# 辅助函数

# 禁用 target 的 clang-tidy
# _target: 要禁用的 target
function(disable_clang_tidy _target)
    message(STATUS "Disabling clang-tidy for ${_target}")
    get_target_property(sources ${_target} SOURCES)
    foreach (source_file ${sources})
        set_source_files_properties(${source_file}
                PROPERTIES
                SKIP_LINTING ON
        )
    endforeach ()
    # SKIP_LINTING 似乎有 bug(cmake v3.27.1)
    set_target_properties(${_target}
            PROPERTIES
            CXX_CLANG_TIDY ""
    )
endfunction()

# 添加 clang-format target
function(format)
    file(GLOB_RECURSE format_sources
            ${CMAKE_SOURCE_DIR}/src/*.h
            ${CMAKE_SOURCE_DIR}/src/*.hpp
            ${CMAKE_SOURCE_DIR}/src/*.c
            ${CMAKE_SOURCE_DIR}/src/*.cpp
            ${CMAKE_SOURCE_DIR}/test/*.h
            ${CMAKE_SOURCE_DIR}/test/*.hpp
            ${CMAKE_SOURCE_DIR}/test/*.c
            ${CMAKE_SOURCE_DIR}/test/*.cpp
    )
    add_custom_target(format
            WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
            COMMENT "Formatting ${format_sources} ..."
            COMMAND ${CLANG_FORMAT_EXE} -i -style=file ${format_sources}
    )
endfunction()
