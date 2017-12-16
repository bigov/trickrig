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
  // вспомогательные структуры для удобства обращения к частям data[] по названиям
  struct Vertex
  {
    struct { GLfloat *x=nullptr, *y=nullptr, *z=nullptr, *w=nullptr; } point;
    struct { GLfloat *r=nullptr, *g=nullptr, *b=nullptr, *a=nullptr; } color;
    struct { GLfloat *x=nullptr, *y=nullptr, *z=nullptr, *w=nullptr; } normal;
    struct { GLfloat *u=nullptr, *v=nullptr;                         } fragm;
  };

  //## Набор данных для формирования в GPU многоугольника с индексацией вершин
  struct snip
  {
    GLsizeiptr data_offset = 0; // смещение начала блока данных в буфере GPU
    GLsizei idx[tr::indices_per_snip] = {0, 1, 2, 2, 3, 0}; // порядок обхода вершин
    Vertex V[tr::vertices_per_snip];

    // Блок данных для передачи в буфер GPU
    GLfloat data[tr::digits_per_snip] = {
   /*|----3D коодината-------|-------RGBA цвет-------|-----вектор нормали------|--текстура----|*/
      0.0f, 0.0f, 0.0f, 1.0f, 0.3f, 0.3f, 0.3f, 1.0f, -.46f, .76f, -.46f, 0.0f, 0.0f,   0.0f,
      0.0f, 0.0f, 1.0f, 1.0f, 0.3f, 0.3f, 0.3f, 1.0f, -.46f, .76f,  .46f, 0.0f, 0.125f, 0.0f,
      1.0f, 0.0f, 1.0f, 1.0f, 0.3f, 0.3f, 0.3f, 1.0f,  .46f, .76f,  .46f, 0.0f, 0.125f, 0.125f,
      1.0f, 0.0f, 0.0f, 1.0f, 0.3f, 0.3f, 0.3f, 1.0f,  .46f, .76f, -.46f, 0.0f, 0.0f,   0.125f,
    };  // нормализованый вектор с небольшим наклоном: .46f, .76f,  .46f

    snip(void);
    snip(const tr::f3d &);
    snip(const snip &);                     // дублирующий конструктор
    snip operator= (const snip &) = delete; // копирующий конструктор - отключен

    void v_reset(void);
    void texture_set(GLfloat, GLfloat);
    void point_set(const tr::f3d &); // установка 3D координат фигуры
    GLsizei *reindex(GLsizei);      // настройка индексов для VBO
    void vbo_append(tr::vbo & VBOdata, tr::vbo & VBOidx);
    void vbo_update(tr::vbo &, tr::vbo &, GLsizeiptr offset);
    void jam_data(vbo &VBOdata, vbo &VBOidx, GLintptr dst);
  };

} //namespace tr

#endif
