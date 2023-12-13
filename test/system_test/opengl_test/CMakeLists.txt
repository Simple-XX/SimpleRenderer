
# This file is a part of Simple-XX/SimplePhysicsEngine
# (https://github.com/Simple-XX/SimplePhysicsEngine).
#
# CMakeLists.txt for Simple-XX/SimplePhysicsEngine.

project(opengl_test)

add_executable(${PROJECT_NAME}
        main.cpp
        display.cpp
)

target_link_libraries(${PROJECT_NAME} PRIVATE
        GLUT::GLUT
        OpenGL::GL
        OpenGL::GLU
)

if (APPLE)
        target_compile_definitions(${PROJECT_NAME} PRIVATE
                GL_SILENCE_DEPRECATION
        )
endif ()
