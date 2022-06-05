
# This file is a part of Simple-XX/SimpleRenderer
# (https://github.com/Simple-XX/SimpleRenderer).
#
# CMakeLists.txt for Simple-XX/SimpleRenderer.

# Set CXX flags for debug
if (CMAKE_BUILD_TYPE STREQUAL DEBUG)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g -ggdb -Wall -Wextra")
elseif (CMAKE_BUILD_TYPE STREQUAL RELEASE)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra -Werror")
else ()
	set(CMAKE_BUILD_TYPE DEBUG)
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g -ggdb -Wall -Wextra")
endif ()

# Set common flags
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O0")

message(STATUS "CMAKE_CXX_FLAGS is ${CMAKE_CXX_FLAGS}")

# gtk
find_package(PkgConfig REQUIRED)
pkg_check_modules(Gtkmm4 REQUIRED gtkmm-4.0)
link_libraries(${Gtkmm4_LIBRARIES})
add_definitions(${Gtkmm4_CFLAGS_OTHER})
link_directories(${Gtkmm4_LIBRARY_DIRS})

# Set include dir
include_directories(
    ./include
    ${Gtkmm4_INCLUDE_DIRS})

aux_source_directory(${CMAKE_CURRENT_SOURCE_DIR} SimpleRender_src)

# 生成可执行文件
ADD_EXECUTABLE(SimpleRender
    ${SimpleRender_src})

target_link_libraries(SimpleRender ${Gtkmm4_LIBRARIES})
