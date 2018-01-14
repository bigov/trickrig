//=============================================================================
//
// file: snip.hpp
//
// Элементы формирования пространства
//
//=============================================================================

#ifndef __SNIP_HPP__
#define __SNIP_HPP__

#include "vbo.hpp"
#include "main.hpp"

namespace tr
{
  // структура для обращения в тексте программы к индексам данных вершин по названиям
  enum SNIP_DATA_ID {
    SNIP_X, SNIP_Y, SNIP_Z, SNIP_W,
    SNIP_R, SNIP_G, SNIP_B, SNIP_A,
    SNIP_NX, SNIP_NY, SNIP_NZ, SNIP_NW,
    SNIP_U, SNIP_V, SNIP_ROW_DIGITS
  };

  //## Набор данных для формирования в GPU многоугольника с индексацией вершин
  struct snip
  {
    GLsizeiptr data_offset = 0; // положение блока данных в буфере GPU

    // Блок данных для передачи в буфер GPU
    // TODO: более универсальным тут было-бы (?) использование контейнера std::vector<GLfloat>,
    GLfloat data[tr::digits_per_snip] = {
   /*|----3D коодината-------|-------RGBA цвет-------|-----вектор нормали------|--текстура----|*/
      0.0f, 0.0f, 0.0f, 1.0f, 0.3f, 0.3f, 0.3f, 1.0f, -.46f, .76f, -.46f, 0.0f, 0.0f,   0.375f,
      0.0f, 0.0f, 1.0f, 1.0f, 0.3f, 0.3f, 0.3f, 1.0f, -.46f, .76f,  .46f, 0.0f, 0.125f, 0.375f,
      1.0f, 0.0f, 1.0f, 1.0f, 0.3f, 0.3f, 0.3f, 1.0f,  .46f, .76f,  .46f, 0.0f, 0.125f, 0.500f,
      1.0f, 0.0f, 0.0f, 1.0f, 0.3f, 0.3f, 0.3f, 1.0f,  .46f, .76f, -.46f, 0.0f, 0.0f,   0.500f,
    };  // нормализованый вектор с небольшим наклоном: .46f, .76f,  .46f

    snip(void) {}
    snip(const snip &);                 // дублирующий конструктор

    snip& operator=(const snip &);      // оператор присваивания
    void copy_data(const snip &);       // копирование данных из другого снипа
    void texture_set(GLfloat, GLfloat); // установка фрагмента текстуры

    // Функции управления данными снипа в буферах VBO данных и VBO индекса
    void vbo_append(const tr::f3d &, tr::vbo &);
    bool vbo_update(const tr::f3d &, tr::vbo &, GLsizeiptr);
    void vbo_jam   (tr::vbo &, GLintptr);
  };

} //namespace tr

#endif
