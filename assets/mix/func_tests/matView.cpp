#include <iostream>
#define GLM_FORCE_RADIANS
#include "glm/glm.hpp"
#include "glm/gtc/constants.hpp"
#include "glm/gtc/integer.hpp"
#include "glm/gtc/matrix_transform.hpp"

int main(int, char**)
{
  glm::vec3
    CameraPosition { 3.1,  2.0, 3.1 },
    CameraTarget   { 3.2,  0.0, 3.2 },
    upVector       { 0.0,  1.0, 0.0 };

  glm::vec4
    dotA { 2.0, 0.0, 2.0, 1.0 },
    dotB { 4.0, 0.0, 2.0, 1.0 },
    dotC { 4.0, 0.0, 4.0, 1.0 },
    dotD { 2.0, 0.0, 4.0, 1.0 };

  // Видовая матрица
  glm::mat4 CameraMatrix = glm::lookAt(
    CameraPosition, // Позиция камеры в мировом пространстве
    CameraTarget,   // Позиция точки, на которую смотрит камера
    upVector        // Направление вверх.
  );

  // Проекционная матрица
  glm::mat4 ProjectionMatrix = glm::perspective(
    glm::radians(64.0f), // Вертикальное поле зрения в радианах. Обычно между 90&deg; (очень широкое) и 30&deg; (узкое)
    1.0f / 1.0f,         // Отношение сторон. Зависит от размеров вашего окна. Заметьте, что 4/3 == 800/600 == 1280/960
    0.1f,                // Ближняя плоскость отсечения. Должна быть больше 0.
    10.0f                // Дальняя плоскость отсечения.
);
  glm::mat4 MVP = /*ProjectionMatrix */ CameraMatrix;

  auto transA = MVP * dotA;
  auto transB = MVP * dotB;
  auto transC = MVP * dotC;
  auto transD = MVP * dotD;

  char buf[256];
  std::sprintf(buf,
    " v    x      z \n"
    " ----------------\n"
    " A: %+4.2f, %+4.2f \n"
    " B: %+4.2f, %+4.2f \n"
    " C: %+4.2f, %+4.2f \n"
    " D: %+4.2f, %+4.2f \n",
      transA.x, transA.z,
      transB.x, transB.z,
      transC.x, transC.z,
      transD.x, transD.z );

  std::cout << "\n" << buf;

  return 0;
}
