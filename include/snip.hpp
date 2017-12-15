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
#include "glsl.hpp"
#include "main.hpp"

namespace tr
{
  //## Привязка адресов данных вершина к именам параметров
  // вспомогательная структура для удобства обращения к частям data[] по названиям
  struct vertex
  {
    GLfloat *data; // адрес начала данных должен передаваться вызывающей функцией
    struct { GLfloat *x, *y, *z, *w; } position;
    struct { GLfloat *r, *g, *b, *a; } color;
    struct { GLfloat *x, *y, *z, *w; } normal;
    struct { GLfloat *u, *v; } fragment;
    vertex(GLfloat *data);
  };

  //## Набор данных для формирования в GPU многоугольника с индексацией вершин
  struct snip
  {
    GLsizeiptr data_offset = 0; // смещение начала блока данных в буфере GPU

    // Блок данных для передачи в буфер GPU
    // (нормализованый вектор с небольшим наклоном: 0.46f, 0.76f, 0.46f)
    GLfloat data[tr::digits_per_snip] = {
   /*|----3D коодината-------|-------RGBA цвет-------|-----вектор нормали------|--текстура----|*/
      0.0f, 0.0f, 0.0f, 1.0f, 0.3f, 0.3f, 0.3f, 1.0f, -.46f, .76f, -.46f, 0.0f, 0.0f,   0.0f,
      0.0f, 0.0f, 1.0f, 1.0f, 0.3f, 0.3f, 0.3f, 1.0f, -.46f, .76f,  .46f, 0.0f, 0.125f, 0.0f,
      1.0f, 0.0f, 1.0f, 1.0f, 0.3f, 0.3f, 0.3f, 1.0f,  .46f, .76f,  .46f, 0.0f, 0.125f, 0.125f,
      1.0f, 0.0f, 0.0f, 1.0f, 0.3f, 0.3f, 0.3f, 1.0f,  .46f, .76f, -.46f, 0.0f, 0.0f,   0.125f,
    };
    GLsizei idx[tr::indices_per_snip] = {0, 1, 2, 2, 3, 0}; // порядок обхода вершин

    // вспомогательная структура для удобства обращения к частям data[] по названиям
    tr::vertex vertices[tr::vertices_per_snip] = {&data[0], &data[14], &data[28], &data[42]};

    snip(void) {}
    snip(const tr::f3d &);          // конструктор по-умолчанию
    void relocate(const tr::f3d &); // установка 3D координат фигуры
    GLsizei *reindex(GLsizei);      // настройка индексов для VBO
    void vbo_append(tr::vbo & VBOdata, tr::vbo & VBOidx);
    void vbo_update(tr::vbo &, tr::vbo &, GLsizeiptr offset);
    void jam_data(vbo &VBOdata, vbo &VBOidx, GLintptr dst);
  };

} //namespace tr

#endif
