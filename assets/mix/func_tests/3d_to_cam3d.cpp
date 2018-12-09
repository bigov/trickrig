#include <iostream>
#define GLM_FORCE_RADIANS
#include "glm/glm.hpp"
#include "glm/gtc/matrix_transform.hpp"

/*
 * преобразование 3D координат точки, на которую направлена
 * камера, в видовые координаты (окна) камеры.
 */

int main(int, char**)
{
  glm::vec3 CamPos3 { 3.0, 2.0, 1.5 };
  glm::vec3 CamDest { 3.0, 0.0, 3.0 };
  glm::vec3 GlobUp  { 0.0, 1.0, 0.0 };

  glm::mat4 View = glm::lookAt(CamPos3, CamDest, GlobUp);

  glm::vec4
    A = View * (glm::vec4 { 2.0, 0.0, 2.0, 1.0 }),
    B = View * (glm::vec4 { 4.0, 0.0, 2.0, 1.0 }),
    C = View * (glm::vec4 { 4.0, 0.0, 4.0, 1.0 }),
    D = View * (glm::vec4 { 2.0, 0.0, 4.0, 1.0 });

  char buf[256];
  std::sprintf(buf,
    "      x      y      z\n"
    " ------------------------\n"
    "  A: %+4.2f, %+4.2f, %+4.2f\n"
    "  B: %+4.2f, %+4.2f, %+4.2f\n"
    "  C: %+4.2f, %+4.2f, %+4.2f\n"
    "  D: %+4.2f, %+4.2f, %+4.2f\n",
      A.x, A.y, A.z,
      B.x, B.y, B.z,
      C.x, C.y, C.z,
      D.x, D.y, D.z );

  std::cout << buf;

  return 0;
}
