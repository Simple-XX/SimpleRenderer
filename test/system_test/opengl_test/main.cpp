
/**
 * @file main.cpp
 * @brief main 实现
 * @author Zone.N (Zone.Niuzh@hotmail.com)
 * @version 1.0
 * @date 2023-10-31
 * @copyright MIT LICENSE
 * https://github.com/Simple-XX/SimplePhysicsEngine
 * @par change log:
 * <table>
 * <tr><th>Date<th>Author<th>Description
 * <tr><td>2023-10-31<td>Zone.N<td>迁移到 doxygen
 * </table>
 */

#include <cmath>

#include <GL/freeglut.h>
#include <GL/gl.h>

#include "display.h"
#include <SimpleRenderer.h>

/// obj 文件目录
static const std::string OBJ_FILE_PATH =
    std::string("/Users/nzh/Documents/SimpleRenderer/obj/");

/// @name 默认大小
/// @{
static constexpr const size_t WIDTH = 1920;
static constexpr const size_t HEIGHT = 1080;
/// @}

int n = 3;

static void DisplayShape(void) {
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
  glColor3d(1, 0.1, 0.6);

  const float x0 = 0.0f;
  const float y0 = 0.0f;
  const float sideLen = 0.5;
  float dist = sideLen / 2.0f / sin(M_PI * 2.0f / n / 2.0f);
  float startAngle = -M_PI * (n - 2) / 2 / n;

  glBegin(GL_LINE_LOOP);
  // n - sides count
  for (int i = 0; i < n; ++i) {
    float sideAngle = M_PI * 2.0 * i / n + startAngle;
    float x = x0 + dist * cos(sideAngle);
    float y = y0 + dist * sin(sideAngle);
    glVertex2f(x, y);
  }
  glEnd();

  glutSwapBuffers();
  glutPostRedisplay();
}

static void key(unsigned char key, int x, int y) {
  switch (key) {
  // quit
  case 27:
  case 'q': {
    exit(0);
    break;
  }
  // increase sides count
  case '1': {
    n++;
    break;
  }
  // decrease sides count
  case '2': {
    // cannot be less than 3
    if (n > 3) {
      n--;
    }
    break;
  }
  }
  glutPostRedisplay();
}

static void reshape(int width, int height) {
  glViewport(0, 0, width, height);
  glMatrixMode(GL_PROJECTION);
  glLoadIdentity();
  double aspect = (double)width / height;
  glOrtho(-aspect, aspect, -1.0, 1.0, -1.0, 1.0);
  glMatrixMode(GL_MODELVIEW);
}



int main(int argc, char *argv[]) {
  // glutInit(&argc, argv);
  // glutInitWindowSize(640, 480);
  // glutInitWindowPosition(10, 10);
  // glutInitDisplayMode(GLUT_RGB | GLUT_DOUBLE | GLUT_DEPTH);

  // glutCreateWindow("GLUT Shapes");

  // glutReshapeFunc(reshape);
  // glutDisplayFunc(DisplayShape);
  // glutKeyboardFunc(key);

  // glutMainLoop();

std::vector<std::string> objs;
  objs.emplace_back(OBJ_FILE_PATH + "cube.obj");
  objs.emplace_back(OBJ_FILE_PATH + "cube2.obj");
  objs.emplace_back(OBJ_FILE_PATH + "cube3.obj");
  objs.emplace_back(OBJ_FILE_PATH + "cornell_box.obj");
  objs.emplace_back(OBJ_FILE_PATH + "helmet.obj");
  objs.emplace_back(OBJ_FILE_PATH + "african_head.obj");
  objs.emplace_back(OBJ_FILE_PATH + "utah-teapot/utah-teapot.obj");

  SimpleRenderer::SimpleRenderer<WIDTH, HEIGHT> render;
  for (auto &obj : objs) {
    render.add_model(obj);
  }

  render.get_buffer();

  auto display = display_t(640, 480);

  return EXIT_SUCCESS;
}
