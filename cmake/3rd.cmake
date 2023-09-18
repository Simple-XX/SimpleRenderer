
# This file is a part of Simple-XX/SimpleRenderer
# (https://github.com/Simple-XX/SimpleRenderer).
#
# 3rd.cmake for Simple-XX/SimpleRenderer.
# 依赖管理

# 设置依赖下载路径
set(CPM_SOURCE_CACHE ${CMAKE_SOURCE_DIR}/3rd)
# 优先使用本地文件
set(CPM_USE_LOCAL_PACKAGES True)
# https://github.com/cpm-cmake/CPM.cmake
# -------- get_cpm.cmake --------
set(CPM_DOWNLOAD_VERSION 0.38.2)

if (CPM_SOURCE_CACHE)
    set(CPM_DOWNLOAD_LOCATION "${CPM_SOURCE_CACHE}/cpm/CPM_${CPM_DOWNLOAD_VERSION}.cmake")
elseif (DEFINED ENV{CPM_SOURCE_CACHE})
    set(CPM_DOWNLOAD_LOCATION "$ENV{CPM_SOURCE_CACHE}/cpm/CPM_${CPM_DOWNLOAD_VERSION}.cmake")
else ()
    set(CPM_DOWNLOAD_LOCATION "${CMAKE_BINARY_DIR}/cmake/CPM_${CPM_DOWNLOAD_VERSION}.cmake")
endif ()

# Expand relative path. This is important if the provided path contains a tilde (~)
get_filename_component(CPM_DOWNLOAD_LOCATION ${CPM_DOWNLOAD_LOCATION} ABSOLUTE)

function(download_cpm)
    message(STATUS "Downloading CPM.cmake to ${CPM_DOWNLOAD_LOCATION}")
    file(DOWNLOAD
            https://github.com/cpm-cmake/CPM.cmake/releases/download/v${CPM_DOWNLOAD_VERSION}/CPM.cmake
            ${CPM_DOWNLOAD_LOCATION}
    )
endfunction()

if (NOT (EXISTS ${CPM_DOWNLOAD_LOCATION}))
    download_cpm()
else ()
    # resume download if it previously failed
    file(READ ${CPM_DOWNLOAD_LOCATION} check)
    if ("${check}" STREQUAL "")
        download_cpm()
    endif ()
    unset(check)
endif ()

include(${CPM_DOWNLOAD_LOCATION})
# -------- get_cpm.cmake --------

# https://github.com/google/googletest
CPMAddPackage(
        NAME googletest
        GITHUB_REPOSITORY google/googletest
        GIT_TAG v1.14.0
        VERSION 1.14.0
        OPTIONS
        "INSTALL_GTEST OFF"
        "gtest_force_shared_crt ON"
)

## https://github.com/abumq/easyloggingpp
#CPMAddPackage(
#        NAME easylogingpp
#        VERSION 9.97.0
#        GITHUB_REPOSITORY amrayn/easyloggingpp
#        OPTIONS
#        "build_static_lib ON"
#        "lib_utc_datetime ON"
#)

# https://github.com/aminosbh/sdl2-cmake-modules.git
CPMAddPackage(
        NAME sdl2-cmake-modules
        GIT_REPOSITORY https://github.com/aminosbh/sdl2-cmake-modules.git
        GIT_TAG ad006a3daae65a612ed87415037e32188b81071e
        DOWNLOAD_ONLY True
)
if (sdl2-cmake-modules_ADDED)
    list(APPEND CMAKE_MODULE_PATH ${sdl2-cmake-modules_SOURCE_DIR})
endif ()

## https://github.com/freetype/freetype
#CPMAddPackage(
#        NAME freetype
#        GIT_REPOSITORY https://github.com/freetype/freetype.git
#        GIT_TAG VER-2-13-0
#        VERSION 2.13.0
#)
#if (freetype_ADDED)
#    add_library(Freetype::Freetype ALIAS freetype)
#endif ()

# https://github.com/tinyobjloader/tinyobjloader.git
CPMAddPackage(
        NAME tinyobjloader
        GIT_REPOSITORY https://github.com/tinyobjloader/tinyobjloader.git
        GIT_TAG 853f059d778058a43c954850e561a231934b33a7
        DOWNLOAD_ONLY True
)
if (tinyobjloader_ADDED)
    add_library(tinyobjloader INTERFACE)
    target_sources(tinyobjloader INTERFACE
            FILE_SET HEADERS
            BASE_DIRS ${tinyobjloader_SOURCE_DIR}
            FILES tiny_obj_loader.h
    )
endif ()

# https://github.com/nothings/stb.git
CPMAddPackage(
        NAME stb
        GIT_REPOSITORY https://github.com/nothings/stb.git
        GIT_TAG 5736b15f7ea0ffb08dd38af21067c314d6a3aae9
        DOWNLOAD_ONLY True
)
if (stb_ADDED)
    add_library(stb INTERFACE)
    target_sources(stb INTERFACE
            FILE_SET HEADERS
            BASE_DIRS ${stb_SOURCE_DIR}
            FILES stb_image.h
    )
endif ()

# https://gitlab.com/libeigen/eigen.git
CPMAddPackage(
        NAME Eigen
        GIT_REPOSITORY https://gitlab.com/libeigen/eigen.git
        GIT_TAG 3.4.0
        VERSION 3.4.0
        DOWNLOAD_ONLY True
)
if (Eigen_ADDED)
    add_library(Eigen INTERFACE IMPORTED)
    target_include_directories(Eigen INTERFACE ${Eigen_SOURCE_DIR})
endif ()

# http://wenq.org/wqy2/index.cgi?ZenHei
CPMAddPackage(
        NAME wqy_font
        URL https://sourceforge.net/projects/wqy/files/wqy-zenhei/0.8.38%20%28Pangu%29/wqy-zenhei-0.8.38-1.tar.gz
        VERSION 0.8.38
        DOWNLOAD_ONLY True
)

# https://github.com/cpm-cmake/CPMLicenses.cmake
# 保持在 CPMAddPackage 的最后
CPMAddPackage(
        NAME CPMLicenses.cmake
        GITHUB_REPOSITORY cpm-cmake/CPMLicenses.cmake
        VERSION 0.0.7
)
if (CPMLicenses.cmake_ADDED)
    cpm_licenses_create_disclaimer_target(
            write-licenses "${CMAKE_CURRENT_SOURCE_DIR}/3rd/LICENSE" "${CPM_PACKAGES}"
    )
endif ()
# make 时自动在 3rd 文件夹下生成 LICENSE 文件
add_custom_target(3rd_licenses
        ALL
        COMMAND
        make
        write-licenses
)

# doxygen
find_package(Doxygen
        REQUIRED dot)
if (NOT DOXYGEN_FOUND)
    message(FATAL_ERROR "Doxygen not found.\n"
            "Following https://www.doxygen.nl/index.html to install.")
endif ()

# cppcheck
find_program(CPPCHECK_EXE NAMES cppcheck)
if (NOT CPPCHECK_EXE)
    message(FATAL_ERROR "cppcheck not found.\n"
            "Following https://cppcheck.sourceforge.io to install.")
endif ()
add_custom_target(cppcheck
        WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
        COMMENT "Run cppcheck on ${CMAKE_BINARY_DIR}/compile_commands.json ..."
        COMMAND
        ${CPPCHECK_EXE}
        --enable=all
        --project=${CMAKE_BINARY_DIR}/compile_commands.json
        --suppress-xml=${CMAKE_SOURCE_DIR}/tools/cppcheck-suppressions.xml
        --output-file=${CMAKE_BINARY_DIR}/cppcheck_report.log
)

# 获取全部源文件
file(GLOB_RECURSE ALL_SOURCE_FILES
        ${CMAKE_SOURCE_DIR}/src/*.h
        ${CMAKE_SOURCE_DIR}/src/*.hpp
        ${CMAKE_SOURCE_DIR}/src/*.c
        ${CMAKE_SOURCE_DIR}/src/*.cpp
        ${CMAKE_SOURCE_DIR}/test/*.h
        ${CMAKE_SOURCE_DIR}/test/*.hpp
        ${CMAKE_SOURCE_DIR}/test/*.c
        ${CMAKE_SOURCE_DIR}/test/*.cpp
)

# clang-tidy
find_program(CLANG_TIDY_EXE NAMES clang-tidy)
if (NOT CLANG_TIDY_EXE)
    message(FATAL_ERROR "clang-tidy not found.\n"
            "Following https://clang.llvm.org/extra/clang-tidy to install.")
endif ()
add_custom_target(clang-tidy
        WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
        COMMENT "Run clang-tidy on ${ALL_SOURCE_FILES} ..."
        COMMAND
        ${CLANG_TIDY_EXE}
        --config-file=${CMAKE_SOURCE_DIR}/.clang-tidy
        -p=${CMAKE_BINARY_DIR}
        ${ALL_SOURCE_FILES}
        > ${CMAKE_BINARY_DIR}/clang_tidy_report.log 2>&1
)

# clang-format
find_program(CLANG_FORMAT_EXE NAMES clang-format)
if (NOT CLANG_FORMAT_EXE)
    message(FATAL_ERROR "clang-format not found.\n"
            "Following https://clang.llvm.org/docs/ClangFormat.html to install.")
endif ()
add_custom_target(clang-format
        WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
        COMMENT "Run clang-format on ${ALL_SOURCE_FILES} ..."
        COMMAND ${CLANG_FORMAT_EXE} -i -style=file ${ALL_SOURCE_FILES}
)

# genhtml 生成测试覆盖率报告网页
find_program(GENHTML_EXE genhtml)
if (NOT GENHTML_EXE)
    message(FATAL_ERROR "genhtml not found.\n"
            "Following https://github.com/linux-test-project/lcov to install.")
endif ()

# lcov 生成测试覆盖率报告
find_program(LCOV_EXE lcov)
if (NOT LCOV_EXE)
    message(FATAL_ERROR "lcov not found.\n"
            "Following https://github.com/linux-test-project/lcov to install.")
endif ()

find_package(SDL2 REQUIRED)
if (NOT SDL2_FOUND)
    message(FATAL_ERROR "sdl2 not found.\n"
            "Following https://github.com/libsdl-org/SDL to install.")
endif ()

find_package(SDL2_ttf REQUIRED)
if (NOT SDL2_TTF_FOUND)
    message(FATAL_ERROR "sdl2_ttf not found.\n"
            "Following https://github.com/libsdl-org/SDL to install.")
endif ()

if (APPLE)
    set(OpenMP_C_FLAGS "-Xpreprocessor -fopenmp")
    set(OpenMP_C_LIB_NAMES "omp")
    set(OpenMP_CXX_FLAGS "-Xpreprocessor -fopenmp")
    set(OpenMP_CXX_LIB_NAMES "omp")
    set(OpenMP_omp_LIBRARY omp)
endif ()
find_package(OpenMP REQUIRED)
if (NOT OpenMP_FOUND)
    message(FATAL_ERROR "OpenMP not found.\n"
            "Following https://www.openmp.org to install.")
endif ()
