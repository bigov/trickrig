/*
 * Выводит список папок в указанном месте. Компилируется командой:
 *
 * \> c++ -std=c++17 dirlist.cpp -lstdc++fs -o d && d
 *
 */
#include <iostream>
#include "glm/glm.hpp"
#include "glm/gtc/constants.hpp"
#include "glm/gtc/integer.hpp"
#include "glm/gtc/matrix_transform.hpp"
#define GLM_ENABLE_EXPERIMENTAL
#include "glm/gtx/rotate_vector.hpp"
#include "glm/gtx/transform.hpp"
#include "glm/gtc/type_ptr.hpp"

using namespace glm;

int main(int, char**)
{

  vec3 A {1, 1, 1};
  vec3 B {1.01, 1, 1};

  std::cout << (A > B) << "\n";

  return 0;
}

