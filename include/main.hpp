//============================================================================
//
// file: main.hpp
//
// Подключаем все внешние модули из одного места
//
//============================================================================
#ifndef MAIN_HPP
#define MAIN_HPP

#include <sys/stat.h>
#include <any>
#include <array>
#include <cassert>
#include <cstdint>
//<cstdio>
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
#include <filesystem>

#include "gl_core33.h"
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

  enum ENUM_INIT {
    TTF_FONT,
    PNG_TEXTURE0,
    PNG_HUD,
    SHADER_VERT_SCENE,
    SHADER_GEOM_SCENE,
    SHADER_FRAG_SCENE,
    SHADER_VERT_SCREEN,
    SHADER_FRAG_SCREEN,
    DB_TPL_FNAME,
    ASSETS_LIST_END,
    WINDOW_SCREEN_FULL,
    WINDOW_WIDTH,
    WINDOW_HEIGHT,
    WINDOW_TOP,
    WINDOW_LEFT,
    INT_LIST_END,
    VIEW_FROM_X,
    VIEW_FROM_Y,
    VIEW_FROM_Z,
    LOOK_AZIM,
    LOOK_TANG,
  };

  extern glm::mat4 MatProjection; // Матрица проекции для рендера 3D-окна

  enum BUTTON_ID {          // Идентификаторы кнопок GIU
    BTN_OPEN,
    BTN_CLOSE,
    NONE
  };

  // Настройка параметров 3D окна
  struct window_gl {
    UINT width = 400;     // ширина окна
    UINT height = 400;    // высота окна
    UINT left = 0;         // положение окна по горизонтали
    UINT top = 0;          // положение окна по вертикали
    float aspect = 1.0f;  // соотношение размеров окна
    bool renew = true;    // флаг наличия изменений параметров окна
    bool is_open = false; // индикатор того, что окно открыто в режиме 3D
    double xpos = 0;      // позиция указателя относительно левой границы
    double ypos = 0;      // позиция указателя относительно верхней границы
    int fps = 120;        // частота кадров (для коррекции скорости движения)
    glm::vec3 Cursor = { 200.5f, 200.5f, .0f }; // x=u, y=v, z - длина прицела

    BUTTON_ID OverButton = NONE; // Над какой кнопкой курсор

    void show_3d(bool state) // Изменение режима окна 3d/2d
    {
      is_open = state;
      Cursor[2] = state ? 4.0f : .0f;
      renew = true;
    }
  };
  extern window_gl WinGl;

  // Настройка параметров главной камеры 3D вида
  struct camera_3d {
    float look_a = 0.0f;       // азимут (0 - X)
    float look_t = 0.0f;       // тангаж (0 - горизОнталь, пи/2 - вертикаль)
    float look_speed = 0.002f; // зависимость угла поворота от сдвига мыши /Config
    float speed = 4.0f;        // корректировка скорости от FPS /Config
    glm::vec3 ViewFrom = {};   // 3D координаты точки положения

    // ID фреймбуфера и его текстуры требуется для call-back функции GLFW, вызываемой
    // при изменениях геометрии окна, отображающего 3D сцену, так как размер текстуры
    // фреймбуфера напрямую связан с размерами окна.
    GLuint frame_buf = 0; // id фрейм-буфера рендера сцены
    GLuint texco_buf = 0; // id тектуры фрейм-буфера
    GLuint rendr_buf = 0; // id рендер-буфера
  };
  extern camera_3d Eye;

  /** Начальная дистанция рендера окружения
   *
   * - блок, над которым расположена камера рендерится всегда, даже при lod_0 = 0.0f
   * - при значении 0.0f < lod_0 <= 1.0f рисуется площадка из 9 блоков
   * - координаты блока (нулевая точка) вычилсяется через floor(), граница - через ceil()
   */
  static const int lod_0 = 25;

  // число вершин в одном снипе
  static const size_t vertices_per_snip = 4;
  // число индексов в одном снипе
  static const size_t indices_per_snip = 6;
  // количество чисел (GLfloat) в блоке данных одной вершины
  static const size_t digits_per_vertex = 14;
  // количество чисел (GLfloat) в блоке данных снипа
  static const size_t digits_per_snip = digits_per_vertex * vertices_per_snip;
  // число элементов в поле shift элемента rig
  //static const size_t digits_per_rig_shift = 7;
  // размер (число байт) блока данных снипа
  static const GLsizeiptr bytes_per_snip = digits_per_snip * sizeof(GLfloat);
  // число байт для записи данных одной вершины
  static const GLsizeiptr bytes_per_vertex = digits_per_vertex * sizeof(GLfloat);

  struct evInput
  {
    float dx, dy;   // смещение указателя мыши в активном окне
    int fb, rl, ud, // управление направлением движения в 3D пространстве
    key_scancode, key_mods, mouse_mods;
  };

  const float pi = glm::pi<glm::float_t>();
  const float two_pi = glm::two_pi<glm::float_t>();
  const float half_pi = glm::half_pi<glm::float_t>();
  const float _half_pi = 0 - half_pi;
  const float look_up = half_pi - 0.01f;
  const float look_down = 0 - half_pi + 0.01f;
  const float three_over_two_pi  = glm::three_over_two_pi<glm::float_t>();

  extern void init_config_db(const std::string &);
}

#endif
