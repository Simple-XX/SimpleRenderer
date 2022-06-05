# Install script for directory: /Users/nzh/Documents/GitHub/SimpleRenderer/demo/加载模型

# Set the install prefix
if(NOT DEFINED CMAKE_INSTALL_PREFIX)
  set(CMAKE_INSTALL_PREFIX "/usr/local")
endif()
string(REGEX REPLACE "/$" "" CMAKE_INSTALL_PREFIX "${CMAKE_INSTALL_PREFIX}")

# Set the install configuration name.
if(NOT DEFINED CMAKE_INSTALL_CONFIG_NAME)
  if(BUILD_TYPE)
    string(REGEX REPLACE "^[^A-Za-z0-9_]+" ""
           CMAKE_INSTALL_CONFIG_NAME "${BUILD_TYPE}")
  else()
    set(CMAKE_INSTALL_CONFIG_NAME "")
  endif()
  message(STATUS "Install configuration: \"${CMAKE_INSTALL_CONFIG_NAME}\"")
endif()

# Set the component getting installed.
if(NOT CMAKE_INSTALL_COMPONENT)
  if(COMPONENT)
    message(STATUS "Install component: \"${COMPONENT}\"")
    set(CMAKE_INSTALL_COMPONENT "${COMPONENT}")
  else()
    set(CMAKE_INSTALL_COMPONENT)
  endif()
endif()

# Is this installation the result of a crosscompile?
if(NOT DEFINED CMAKE_CROSSCOMPILING)
  set(CMAKE_CROSSCOMPILING "FALSE")
endif()

# Set default install directory permissions.
if(NOT DEFINED CMAKE_OBJDUMP)
  set(CMAKE_OBJDUMP "/Applications/Xcode.app/Contents/Developer/Toolchains/XcodeDefault.xctoolchain/usr/bin/objdump")
endif()

if("x${CMAKE_INSTALL_COMPONENT}x" STREQUAL "xUnspecifiedx" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/lib" TYPE EXECUTABLE FILES "/Users/nzh/Documents/GitHub/SimpleRenderer/demo/加载模型/SIMPLERENDER")
  if(EXISTS "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib/SIMPLERENDER" AND
     NOT IS_SYMLINK "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib/SIMPLERENDER")
    execute_process(COMMAND /usr/bin/install_name_tool
      -delete_rpath "/usr/local/Cellar/gtkmm4/4.2.0/lib"
      "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib/SIMPLERENDER")
    execute_process(COMMAND /usr/bin/install_name_tool
      -delete_rpath "/usr/local/Cellar/pangomm/2.48.1/lib"
      "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib/SIMPLERENDER")
    execute_process(COMMAND /usr/bin/install_name_tool
      -delete_rpath "/usr/local/Cellar/glibmm/2.70.0/lib"
      "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib/SIMPLERENDER")
    execute_process(COMMAND /usr/bin/install_name_tool
      -delete_rpath "/usr/local/Cellar/cairomm/1.16.1/lib"
      "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib/SIMPLERENDER")
    execute_process(COMMAND /usr/bin/install_name_tool
      -delete_rpath "/usr/local/Cellar/libsigc++/3.0.7/lib"
      "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib/SIMPLERENDER")
    execute_process(COMMAND /usr/bin/install_name_tool
      -delete_rpath "/usr/local/Cellar/gtk4/4.4.1/lib"
      "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib/SIMPLERENDER")
    execute_process(COMMAND /usr/bin/install_name_tool
      -delete_rpath "/usr/local/Cellar/pango/1.48.10/lib"
      "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib/SIMPLERENDER")
    execute_process(COMMAND /usr/bin/install_name_tool
      -delete_rpath "/usr/local/Cellar/harfbuzz/3.0.0/lib"
      "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib/SIMPLERENDER")
    execute_process(COMMAND /usr/bin/install_name_tool
      -delete_rpath "/usr/local/Cellar/gdk-pixbuf/2.42.6/lib"
      "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib/SIMPLERENDER")
    execute_process(COMMAND /usr/bin/install_name_tool
      -delete_rpath "/usr/local/Cellar/cairo/1.16.0_5/lib"
      "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib/SIMPLERENDER")
    execute_process(COMMAND /usr/bin/install_name_tool
      -delete_rpath "/usr/local/Cellar/graphene/1.10.6/lib"
      "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib/SIMPLERENDER")
    execute_process(COMMAND /usr/bin/install_name_tool
      -delete_rpath "/usr/local/Cellar/glib/2.70.1/lib"
      "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib/SIMPLERENDER")
    execute_process(COMMAND /usr/bin/install_name_tool
      -delete_rpath "/usr/local/opt/gettext/lib"
      "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib/SIMPLERENDER")
    if(CMAKE_INSTALL_DO_STRIP)
      execute_process(COMMAND "/Applications/Xcode.app/Contents/Developer/Toolchains/XcodeDefault.xctoolchain/usr/bin/strip" -u -r "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib/SIMPLERENDER")
    endif()
  endif()
endif()

if(CMAKE_INSTALL_COMPONENT)
  set(CMAKE_INSTALL_MANIFEST "install_manifest_${CMAKE_INSTALL_COMPONENT}.txt")
else()
  set(CMAKE_INSTALL_MANIFEST "install_manifest.txt")
endif()

string(REPLACE ";" "\n" CMAKE_INSTALL_MANIFEST_CONTENT
       "${CMAKE_INSTALL_MANIFEST_FILES}")
file(WRITE "/Users/nzh/Documents/GitHub/SimpleRenderer/demo/加载模型/${CMAKE_INSTALL_MANIFEST}"
     "${CMAKE_INSTALL_MANIFEST_CONTENT}")
