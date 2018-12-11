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

  using u_char = unsigned char;
  using u_int  = unsigned int;
  using u_long = unsigned long;

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
  extern glm::mat4 MatMVP;        // Матрица преобразования

  enum BUTTON_ID { // Идентификаторы кнопок GIU
    BTN_OPEN,
    BTN_CLOSE,
    BTN_CONFIG,
    BTN_LOCATION,
    BTN_CREATE,
    BTN_ENTER_NAME,
    NONE
  };

  enum GUI_MODE_ID {   // режимы окна
    GUI_HUD3D,         // основной режим - без шторки
    GUI_MENU_START,    // начальное меню
    GUI_MENU_LSELECT,  // выбор игры
    GUI_MENU_CREATE,   // создание нового района
    GUI_MENU_CONFIG,   // настройки
  };

  // Настройка значений параметров для сравнения mouse_button и mouse_action
  // будут выполнены в классе управления окном
  extern int MOUSE_BUTTON_LEFT;  // GLFW_MOUSE_BUTTON_LEFT
  extern int MOUSE_BUTTON_RIGHT; // GLFW_MOUSE_BUTTON_RIGHT
  extern int PRESS;              // GLFW_PRESS
  extern int RELEASE;            // GLFW_RELEASE

  extern int KEY_ESCAPE;         // GLFW_KEY_ESCAPE
  extern int KEY_BACKSPACE;      // GLFW_KEY_BACKSPACE

  // Параметры и режимы окна приложения
  struct main_window {
    u_int width = 400;              // ширина окна
    u_int height = 400;             // высота окна
    u_int left = 0;                 // положение окна по горизонтали
    u_int top = 0;                  // положение окна по вертикали
    u_int btn_w = 120;              // ширина кнопки GUI
    u_int btn_h = 36;               // высота кнопки GUI
    u_int minwidth = btn_w + 8;     // минимально допустимая ширина окна
    u_int minheight = btn_h * 4 + 8;// минимально допустимая высота окна
    GUI_MODE_ID gui_mode = GUI_MENU_START; // режим окна приложения
    std::wstring* input_buffer = nullptr;  // строка ввода пользователя

    bool run        = true;         // индикатор закрытия окна
    float aspect = 1.0f;  // соотношение размеров окна
    bool resized = true;  // флаг наличия изменений параметров окна
    double xpos = 0.0;    // позиция указателя относительно левой границы
    double ypos = 0.0;    // позиция указателя относительно верхней границы
    int fps = 120;        // частота кадров (для коррекции скорости движения)
    glm::vec3 Cursor = { 200.5f, 200.5f, .0f }; // x=u, y=v, z - длина прицела

    BUTTON_ID ButtonOver = NONE;      // Над какой GIU кнопкой курсор

    int key    = -1;
    int mouse  = -1;
    int action = -1;

    char set_mouse_ptr = 0;           // запрос смены типа курсора {-1, 0, 1}
  };
  extern main_window AppWin;

  // Настройка параметров главной камеры 3D вида
  struct camera_3d {
    float look_a = 0.0f;       // азимут (0 - X)
    float look_t = 0.0f;       // тангаж (0 - горизОнталь, пи/2 - вертикаль)
    float look_speed = 0.002f; // зависимость угла поворота от сдвига мыши /Config
    float speed = 2.0f;        // корректировка скорости от FPS /Config
    glm::vec3 ViewFrom = {};   // 3D координаты точки положения

    // ID фреймбуфера и его текстуры требуется для call-back функции GLFW, вызываемой
    // при изменениях геометрии окна, отображающего 3D сцену, так как размер текстуры
    // фреймбуфера напрямую связан с размерами окна.
    GLuint framebuf = 0;   // id фрейм-буфера рендера сцены
    GLuint fb_text_0 = 0;  //  id основной тектуры фрейм-буфера
    GLuint fb_text_1 = 0;  //  id тектуры фрейм-буфера для идентификации объектов
    GLuint rendr_buf = 0;  // id рендер-буфера
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

  extern void init_config_db(const std::string &);
}

#endif
