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
#include <sstream>
#include <string>
#include <valarray>
#include <vector>
#include <unordered_map>
#include <utility>
#include <filesystem>

#include "glad.h"
#define GLM_FORCE_RADIANS
#include "glm/glm.hpp"
#include "glm/gtc/constants.hpp"
#include "glm/gtc/integer.hpp"
#include "glm/gtc/matrix_transform.hpp"
#define GLM_ENABLE_EXPERIMENTAL
#include "glm/gtx/rotate_vector.hpp"
#include "glm/gtx/transform.hpp"
#include "glm/gtc/type_ptr.hpp"

#include "GLFW/glfw3.h"

#define ERR throw std::runtime_error
namespace fs = std::filesystem;

namespace tr {

extern std::string AppPathDir; // путь размещения программы

using u_char = unsigned char;
using u_int  = unsigned int;
using u_long = unsigned long;
using v_str  = std::vector<std::string>;
using v_ch   = std::vector<char>;
using v_uch  = std::vector<unsigned char>;
using v_fl   = std::vector<float>;
using v_flp  = std::vector<float*>;
using a_f2   = std::array<float, 2>;
using a_f3   = std::array<float, 3>;
using a_f4   = std::array<float, 4>;
using a_uch4 = std::array<unsigned char, 4>;
using a_int3 = std::array<int, 3>;

enum APP_INIT {      // вначале списка идут названия файлов
  PNG_TEXTURE0,
  DB_TPL_FNAME,
  SHADER_VERT_SCENE,
  SHADER_GEOM_SCENE,
  SHADER_FRAG_SCENE,
  SHADER_VERT_SCREEN,
  SHADER_FRAG_SCREEN,
  ASSETS_LIST_END,      // конец списка файлов - далее только параметры
  WINDOW_SCREEN_FULL,
  WINDOW_WIDTH,
  WINDOW_HEIGHT,
  WINDOW_TOP,
  WINDOW_LEFT,
  APP_INIT_SIZE
};

enum MAP_INIT {
  VIEW_FROM_X,
  VIEW_FROM_Y,
  VIEW_FROM_Z,
  LOOK_AZIM,
  LOOK_TANG,
  MAP_NAME,              // имя карты, присвоеное при создании
  MAP_INIT_SIZE
};

// структура для обращения в тексте программы к индексам данных вершин по названиям
enum SIDE_DATA_ID { X, Y, Z, R, G, B, A, NX, NY, NZ, U, V, SIDE_DATA_SIZE };

extern glm::mat4 MatProjection; // Матрица проекции для рендера 3D-окна
extern float zNear;
extern float zFar;
extern glm::mat4 MatMVP;        // Матрица преобразования

// Настройка значений параметров для сравнения mouse_button и mouse_action
// будут выполнены в классе управления окном
extern const int MOUSE_BUTTON_LEFT;  // GLFW_MOUSE_BUTTON_LEFT
extern const int MOUSE_BUTTON_RIGHT; // GLFW_MOUSE_BUTTON_RIGHT
extern const int PRESS;              // GLFW_PRESS
extern const int REPEAT;             // GLFW_REPEAT
extern const int RELEASE;            // GLFW_RELEASE
extern const int KEY_ESCAPE;         // GLFW_KEY_ESCAPE
extern const int KEY_BACKSPACE;      // GLFW_KEY_BACKSPACE

  // Настройка параметров главной камеры 3D вида
  struct camera_3d {
    float look_a = 0.0f;       // азимут (0 - X)
    float look_t = 0.0f;       // тангаж (0 - горизОнталь, пи/2 - вертикаль)
    float look_speed = 0.002f; // зависимость угла поворота от сдвига мыши /Config
    float speed = 2.0f;        // корректировка скорости от FPS /Config
    glm::vec3 ViewFrom = {};   // 3D координаты точки положения
  };
  extern camera_3d Eye;

  // число вершин в прямоугольнике
  static const u_int vertices_per_side = 4;

  // число индексов в одном снипе
  static const u_int indices_per_side = 6;

  // количество чисел (GLfloat) в блоке данных одной вершины
  static const size_t digits_per_vertex = 12;

  // количество чисел (GLfloat) в блоке данных прямоугольника
  static const size_t digits_per_side = digits_per_vertex * vertices_per_side;

  // количество чисел (GLfloat) в блоке данных вокселя
  static const size_t digits_per_voxel = digits_per_side * 6;

  // размер (число байт) блока данных одной стороны вокселя
  static const GLsizeiptr bytes_per_side = digits_per_side * sizeof(GLfloat);

  // число байт для записи данных одной вершины
  static const GLsizeiptr bytes_per_vertex = digits_per_vertex * sizeof(GLfloat);

  static const char fname_cfg[] = "config.db";
  static const char fname_map[] = "map.db";

  struct evInput
  {
    float dx, dy;   // смещение указателя мыши в активном окне
    int fb, rl, ud, // управление направлением движения в 3D пространстве
    scancode, mods, mouse, action, key;
  };

  struct texture {
      GLfloat u = 0.0f;
      GLfloat v = 0.0f;
  };

  struct normal {
      float nx = 0.0f;
      float ny = 0.0f;
      float nz = 0.0f;
  };

  struct color {
      float r = 1.0f;
      float g = 1.0f;
      float b = 1.0f;
      float a = 1.0f;
  };

  // структуры для оперирования опорными точками в пространстве трехмерных координат
  struct i3d
  {
    int x, y, z;
    i3d(void) = delete;

    i3d(int X, int Y, int Z): x(X), y(Y), z(Z) {}
    i3d(const glm::vec3 &v): x(static_cast<int>(floor(v.x))),
      y(static_cast<int>(floor(v.y))), z(static_cast<int>(floor(v.z))) {}
    i3d(const glm::vec4 &v): x(static_cast<int>(floor(v.x))),
      y(static_cast<int>(floor(v.y))), z(static_cast<int>(floor(v.z))) {}
  };
  extern bool operator< (i3d const& left, i3d const& right);

  struct f3d
  {
    float x = 0.f, y = 0.f, z = 0.f;

    f3d(void) {};
    // конструкторы для обеспечения инициализации разными типами данных
    f3d(float x, float y, float z): x(x), y(y), z(z) {}

    f3d(double x, double y, double z): x(static_cast<float>(x)),
                                       y(static_cast<float>(y)),
                                       z(static_cast<float>(z)) {}

    f3d(const i3d& P): x(static_cast<float>(P.x)),
                       y(static_cast<float>(P.y)),
                       z(static_cast<float>(P.z)) {}

    f3d(int x, int y, int z): x(static_cast<float>(x)),
                              y(static_cast<float>(y)),
                              z(static_cast<float>(z)) {}

    f3d(glm::vec3 v): x(v.x), y(v.y), z(v.z) {}
  };

}

#endif
