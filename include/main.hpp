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

#include "i_win.hpp"

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

struct layout
{
  u_int width  = 0;
  u_int height = 0;
  u_int left = 0;
  u_int top = 0;
};

// структура для обращения в тексте программы к индексам данных вершин по названиям
enum SIDE_DATA_ID { X, Y, Z, R, G, B, A, NX, NY, NZ, U, V, SIDE_DATA_SIZE };

// Настройка значений параметров для сравнения mouse_button и mouse_action
// будут выполнены в классе управления окном
const int MOUSE_BUTTON_LEFT  = GLFW_MOUSE_BUTTON_LEFT;
const int MOUSE_BUTTON_RIGHT = GLFW_MOUSE_BUTTON_RIGHT;
const int PRESS              = GLFW_PRESS;
const int REPEAT             = GLFW_REPEAT;
const int RELEASE            = GLFW_RELEASE;
const int KEY_ESCAPE         = GLFW_KEY_ESCAPE;
const int KEY_BACKSPACE      = GLFW_KEY_BACKSPACE;

const int KEY_MOVE_FRONT = GLFW_KEY_W;
const int KEY_MOVE_BACK  = GLFW_KEY_S;
const int KEY_MOVE_UP    = GLFW_KEY_LEFT_SHIFT;
const int KEY_MOVE_DOWN  = GLFW_KEY_SPACE;
const int KEY_MOVE_RIGHT = GLFW_KEY_D;
const int KEY_MOVE_LEFT  = GLFW_KEY_A;

  // Настройка параметров главной камеры 3D вида
  struct camera_3d {
    float look_a = 0.0f;       // азимут (0 - X)
    float look_t = 0.0f;       // тангаж (0 - горизОнталь, пи/2 - вертикаль)

    // TODO: измерять средний за 10 сек. fps, и пропорционально менять скорость перемещения
    float speed_rotate = 0.001f; // скорость поворота (радиан в секунду) камеры
    float speed_moving = 10.f;   // скорость перемещения (в секунду) камеры

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
    i3d(void): x(0), y(0), z(0) {}
    i3d(int X, int Y, int Z): x(X), y(Y), z(Z) {}
  };
  extern bool operator== (const i3d&, const i3d&);

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
