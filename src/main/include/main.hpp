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
//#include <any>
#include <array>
#include <atomic>
#include <bitset>
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
#include <mutex>
#include <sstream>
#include <string>
#include <thread>
#include <valarray>
#include <vector>
#include <unordered_map>
#include <utility>
#include <filesystem>

#define GLM_FORCE_RADIANS
#include <glm/glm.hpp>
#include <glm/gtc/constants.hpp>
#include <glm/gtc/integer.hpp>
#include <glm/gtc/matrix_transform.hpp>
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/rotate_vector.hpp>
#include <glm/gtx/transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "glad/glad.h"
#include "i_win.hpp"
#include "version.hpp"

#define ERR throw std::runtime_error
namespace fs = std::filesystem;

namespace tr {

#define SIDE_XP       0
#define SIDE_XN       1
#define SIDE_YP       2
#define SIDE_YN       3
#define SIDE_ZP       4
#define SIDE_ZN       5
#define SIDES_COUNT   6

// Взаимодействие потоков
extern std::atomic<GLsizei> render_indices;
extern std::atomic<int> click_side_vertex_id; // верхний индекс вершины выделенной стороны
extern std::mutex view_mtx;  // Доступ к положению камеры
extern std::mutex vbo_mtx;   // Доступ к буферу вершин
extern std::mutex log_mtx;   // Доступ к записи в журнал

using uchar = unsigned char;
using uint  = unsigned int;
using ulong = unsigned long;
using v_str = std::vector<std::string>;
using v_ch  = std::vector<char>;

enum APP_INIT {
  PNG_TEXTURE0,   // вначале списка идут названия файлов
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
  APP_VER_MAJOR,
  APP_VER_MINOR,
  APP_VER_PATCH,
  APP_VER_TWEAK,
  APP_INIT_SIZE  // размер списка
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

// Хранение данных положения и размера прямоугольника (используется окном)
struct layout
{
  uint width  = 0;
  uint height = 0;
  uint left = 0;
  uint top = 0;
};

// Обмен данными между glsl-программой и VBO
struct glsl_attributes
{
  GLuint index;
  GLint d_size;
  GLenum type;
  GLboolean normalized;
  GLsizei stride;
  size_t pointer;
};


// структура для обращения в тексте программы к индексам данных вершин по названиям
enum SIDE_DATA_ID { X, Y, Z, R, G, B, A, NX, NY, NZ, U, V, SIDE_DATA_SIZE };

const int sizeof_y = sizeof (int); // размер (в байтах) координаты Y вокса

// Настройка значений параметров для сравнения mouse_button и mouse_action
// будут выполнены в классе управления окном
const int MOUSE_BUTTON_LEFT  = GLFW_MOUSE_BUTTON_LEFT;
const int MOUSE_BUTTON_RIGHT = GLFW_MOUSE_BUTTON_RIGHT;
const int PRESS              = GLFW_PRESS;
const int REPEAT             = GLFW_REPEAT;
const int RELEASE            = GLFW_RELEASE;
const int KEY_ESCAPE         = GLFW_KEY_ESCAPE;
const int KEY_BACKSPACE      = GLFW_KEY_BACKSPACE;
const int EMPTY              = -1;

const int KEY_MOVE_FRONT = GLFW_KEY_W;
const int KEY_MOVE_BACK  = GLFW_KEY_S;
const int KEY_MOVE_UP    = GLFW_KEY_LEFT_SHIFT;
const int KEY_MOVE_DOWN  = GLFW_KEY_SPACE;
const int KEY_MOVE_RIGHT = GLFW_KEY_D;
const int KEY_MOVE_LEFT  = GLFW_KEY_A;

// LOD control
const int size_v4 = 32;         // размер стороны вокселя
const int border_dist_b4 = 24;  // число элементов от камеры до отображаемой границы

// число вершин в прямоугольнике
static const int vertices_per_face = 4;

// число индексов в одном снипе
static const int indices_per_face = 6;

// количество чисел (GLfloat) в блоке данных одной вершины
static const size_t digits_per_vertex = 12;

// количество чисел (GLfloat) в блоке данных прямоугольника
static const size_t digits_per_face = digits_per_vertex * vertices_per_face;

// количество чисел (GLfloat) в блоке данных вокселя
static const size_t digits_per_vox = digits_per_face * 6;

// размер (число байт) блока данных одной стороны вокселя
static const GLsizeiptr bytes_per_face = digits_per_face * sizeof(GLfloat);

// число байт для записи данных одной вершины
static const GLsizeiptr bytes_per_vertex = digits_per_vertex * sizeof(GLfloat);

static const char fname_cfg[] = "config.db";
static const char fname_map[] = "map.db";

struct normal      { float nx = 0.0f, ny = 0.0f, nz = 0.0f; };
struct float_color {         float r = 1.0f, g = 1.0f, b = 1.0f, a = 1.0f; };
struct uchar_color { unsigned char r = 0xff, g = 0xff, b = 0xff, a = 0xff; };

// структуры для оперирования опорными точками в пространстве трехмерных координат
struct i3d
{
  int x = 0;
  int y = 0;
  int z = 0;
};

extern bool operator== (const i3d&, const i3d&);

}

#endif
