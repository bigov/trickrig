//============================================================================
//
// file: main.hpp
//
// Подключаем все внешние модули из одного места
//
//============================================================================
#ifndef __MAIN_HPP__
#define __MAIN_HPP__

#include <sys/stat.h>
#include <array>
#include <cassert>
#include <cstdint>
#include <cstring>
#include <chrono>
#include <forward_list>
#include <fstream>
#include <functional>
#include <iomanip>
#include <iostream>
#include <list>
#include <locale>
#include <memory>
#include <map>
#include <random>
#include <sstream>
#include <string>
#include <valarray>
#include <vector>
#include <unordered_map>
#include <utility>

#include "gl_core33.h"
#include "GLFW/glfw3.h"
#include "png.h"
#include "ft2build.h"
#include FT_FREETYPE_H
#include FT_GLYPH_H

#define GLM_FORCE_RADIANS
#include "glm/glm.hpp"
#include "glm/gtc/constants.hpp"
#include "glm/gtc/integer.hpp"
#include "glm/gtc/matrix_transform.hpp"
#define GLM_ENABLE_EXPERIMENTAL
#include "glm/gtx/rotate_vector.hpp"
#include "glm/gtx/transform.hpp"
#include "glm/gtc/type_ptr.hpp"

#define ERR throw std::runtime_error

namespace tr {

  /** Начальная дистанция рендера окружения
   *
   * - блок, над которым расположена камера отображается всегда, даже при lod_0 = 0.0f
   * - при значении 0.0f < lod_0 <= 1.0f рисуется площадка из 9 блоков
   * - координаты блока (нулевая точка) вычилсяется через floor(), граница - через ceil()
   */
  static const float lod_0 = 4.0f;

  // число вершин в одном снипе
  static const size_t vertices_per_snip = 4;
  // число индексов в одном снипе
  static const size_t indices_per_snip = 6;
  // количество чисел (GLfloat) в блоке данных одной вершины
  static const size_t digits_per_vertex = 14;
  // количество чисел (GLfloat) в блоке данных снипа
  static const size_t digits_per_snip = digits_per_vertex * vertices_per_snip;
  // размер (число байт) блока данных снипа
  static const GLsizeiptr snip_data_bytes = digits_per_snip * sizeof(GLfloat);
  // размер (число байт) блока индексов снипа
  static const GLsizeiptr snip_index_bytes = indices_per_snip * sizeof(GLuint);
  // число байт для записи данных одной вершины
  static const GLsizeiptr snip_bytes_per_vertex = digits_per_vertex * sizeof(GLfloat);

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
