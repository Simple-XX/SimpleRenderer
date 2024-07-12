# This file is a part of Simple-XX/SimpleRenderer (https://github.com/Simple-XX/SimpleRenderer).
# 3rd.cmake for Simple-XX/SimpleRenderer. Dependency management.

# Set dependency download path
set(CPM_SOURCE_CACHE ${CMAKE_SOURCE_DIR}/3rd)
set(CPM_USE_LOCAL_PACKAGES True)
set(CPM_DOWNLOAD_VERSION 0.38.2)

if (CPM_SOURCE_CACHE)
    set(CPM_DOWNLOAD_LOCATION "${CPM_SOURCE_CACHE}/cpm/CPM_${CPM_DOWNLOAD_VERSION}.cmake")
elseif (DEFINED ENV{CPM_SOURCE_CACHE})
    set(CPM_DOWNLOAD_LOCATION "$ENV{CPM_SOURCE_CACHE}/cpm/CPM_${CPM_DOWNLOAD_VERSION}.cmake")
else ()
    set(CPM_DOWNLOAD_LOCATION "${CMAKE_BINARY_DIR}/cmake/CPM_${CPM_DOWNLOAD_VERSION}.cmake")
endif ()

# Expand relative path
get_filename_component(CPM_DOWNLOAD_LOCATION ${CPM_DOWNLOAD_LOCATION} ABSOLUTE)

# Download CPM.cmake if it doesn't exist
if (NOT (EXISTS ${CPM_DOWNLOAD_LOCATION}))
    file(DOWNLOAD
        https://github.com/cpm-cmake/CPM.cmake/releases/download/v${CPM_DOWNLOAD_VERSION}/CPM.cmake
        ${CPM_DOWNLOAD_LOCATION}
    )
endif ()

include(${CPM_DOWNLOAD_LOCATION})

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

CPMAddPackage(
    NAME raylib 
    GITHUB_REPOSITORY raysan5/raylib 
    GIT_TAG 5.0 )

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

# https://github.com/TheLartians/PackageProject.cmake
CPMAddPackage("gh:TheLartians/PackageProject.cmake@1.11.0")
CPMAddPackage("gh:cpm-cmake/CPMLicenses.cmake@0.0.7")

# Create disclaimer target for licenses
if (CPMLicenses.cmake_ADDED)
    cpm_licenses_create_disclaimer_target(write-licenses "${CMAKE_CURRENT_SOURCE_DIR}/3rd/LICENSE" "${CPM_PACKAGES}")
endif ()

add_custom_target(3rd_licenses ALL COMMAND make write-licenses)

# Find and check dependencies
find_package(Doxygen REQUIRED dot)
if (NOT DOXYGEN_FOUND)
    message(FATAL_ERROR "Doxygen not found. Please install Doxygen.")
endif ()

find_program(CPPCHECK_EXE NAMES cppcheck)
if (NOT CPPCHECK_EXE)
    message(FATAL_ERROR "cppcheck not found. Please install cppcheck.")
endif ()

add_custom_target(cppcheck
    WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
    COMMENT "Run cppcheck on ${CMAKE_BINARY_DIR}/compile_commands.json ..."
    COMMAND ${CPPCHECK_EXE}
    --enable=all
    --project=${CMAKE_BINARY_DIR}/compile_commands.json
    --suppress-xml=${CMAKE_SOURCE_DIR}/tools/cppcheck-suppressions.xml
    --output-file=${CMAKE_BINARY_DIR}/cppcheck_report.log
)

# Collect all source files for static analysis
file(GLOB_RECURSE ALL_SOURCE_FILES
    ${CMAKE_SOURCE_DIR}/src/*.[ch]pp
    ${CMAKE_SOURCE_DIR}/src/*.[ch]
    ${CMAKE_SOURCE_DIR}/test/*.[ch]pp
    ${CMAKE_SOURCE_DIR}/test/*.[ch]
)

# clang-tidy
find_program(CLANG_TIDY_EXE NAMES clang-tidy)
if (NOT CLANG_TIDY_EXE)
    message(FATAL_ERROR "clang-tidy not found. Please install clang-tidy.")
endif ()

add_custom_target(clang-tidy
    WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
    COMMENT "Run clang-tidy on ${ALL_SOURCE_FILES} ..."
    COMMAND ${CLANG_TIDY_EXE}
    --config-file=${CMAKE_SOURCE_DIR}/.clang-tidy
    -p=${CMAKE_BINARY_DIR}
    ${ALL_SOURCE_FILES}
    > ${CMAKE_BINARY_DIR}/clang_tidy_report.log 2>&1
)

# clang-format
find_program(CLANG_FORMAT_EXE NAMES clang-format)
if (NOT CLANG_FORMAT_EXE)
    message(FATAL_ERROR "clang-format not found. Please install clang-format.")
endif ()

add_custom_target(clang-format
    WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
    COMMENT "Run clang-format on ${ALL_SOURCE_FILES} ..."
    COMMAND ${CLANG_FORMAT_EXE} -i -style=file ${ALL_SOURCE_FILES}
)

# Configure OpenMP for macOS
if (APPLE)
    set(OPENMP_LIBRARIES "/opt/homebrew/opt/libomp/lib")
    set(OPENMP_INCLUDES "/opt/homebrew/opt/libomp/include")

    if (CMAKE_C_COMPILER_ID MATCHES "Clang")
        set(OpenMP_C "${CMAKE_C_COMPILER}")
        set(OpenMP_C_FLAGS "-fopenmp=libomp -Wno-unused-command-line-argument")
        set(OpenMP_C_LIB_NAMES "omp")
        set(OpenMP_omp_LIBRARY ${OPENMP_LIBRARIES}/libomp.dylib)
    endif()

    if (CMAKE_CXX_COMPILER_ID MATCHES "Clang")
        set(OpenMP_CXX "${CMAKE_CXX_COMPILER}")
        set(OpenMP_CXX_FLAGS "-fopenmp=libomp -Wno-unused-command-line-argument")
        set(OpenMP_CXX_LIB_NAMES "omp")
        set(OpenMP_omp_LIBRARY ${OPENMP_LIBRARIES}/libomp.dylib)
    endif()

    find_package(OpenMP REQUIRED)
    if (OPENMP_FOUND)
        include_directories("${OPENMP_INCLUDES}")
        link_directories("${OPENMP_LIBRARIES}")
        set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${OpenMP_C_FLAGS}")
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${OpenMP_CXX_FLAGS}")
        set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -L${OPENMP_LIBRARIES} -lomp")
    endif ()
else ()
    find_package(OpenMP REQUIRED)
    if (NOT OpenMP_FOUND)
        message(FATAL_ERROR "OpenMP not found. Please install OpenMP.")
    endif ()
endif ()

# Find OpenGL
find_package(OpenGL REQUIRED)
if (OPENGL_FOUND)
    include_directories(${OPENGL_INCLUDE_DIR})
    link_libraries(${OPENGL_LIBRARIES})
else ()
    message(FATAL_ERROR "OpenGL not found. Please install OpenGL.")
endif ()

# Find spdlog
find_package(spdlog REQUIRED)
if (NOT spdlog_FOUND)
    message(FATAL_ERROR "spdlog not found. Please install spdlog.")
endif ()
