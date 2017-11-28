//============================================================================
//
// file: main.hpp
//
// Подключаем все внешние модули из одного места
//
//============================================================================
#ifndef __MAIN_HPP__
#define __MAIN_HPP__

//#define NDEBUG // в релизе все проверки можно отключить
#include <sys/stat.h>
#include <cassert>
#include <cstdint>
#include <cstring>
#include <chrono>
#include <memory>
#include <map>
#include <iostream>
#include <fstream>
#include <string>
#include <list>
#include <locale>
#include <vector>
#include <valarray>
#include <random>
#include <unordered_map>

#include "gl_core33.h"
#include <GLFW/glfw3.h>
#include <png.h>
#include <ft2build.h>
#include FT_FREETYPE_H
#include FT_GLYPH_H

#define GLM_FORCE_RADIANS
#include <glm/glm.hpp>
#include <glm/gtc/constants.hpp>
#include <glm/gtc/integer.hpp>
#include <glm/gtc/matrix_transform.hpp>
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/rotate_vector.hpp>
#include <glm/gtx/transform.hpp>
#include <glm/gtc/type_ptr.hpp>


#define ERR throw std::runtime_error

// Длина стороны пространства (должна быть нечетным числом)
#define WIDTH_0 31

namespace tr {

  struct evInput
  {
    float dx, dy;   // смещение указателя мыши в активном окне
    int fb, rl, ud, // управление направлением движения в 3D пространстве
    key_scancode, key_mods, mouse_mods,
    fps; // частота кадров (для коррекции скорости движения)
  };

  const   float pi = glm::pi<glm::float_t>();
  const   float two_pi = glm::two_pi<glm::float_t>();
  const   float half_pi = glm::half_pi<glm::float_t>();
  const   float _half_pi = 0 - half_pi;
  const   float look_up = half_pi - 0.01f;
  const   float look_down = 0 - half_pi + 0.01f;
  const   float three_over_two_pi  = glm::three_over_two_pi<glm::float_t>();
}

#endif
