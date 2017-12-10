//=============================================================================
//
// file: rigs.hpp
//
// Элементы формирования пространства
//
//=============================================================================
#ifndef __RIGS_HPP__
#define __RIGS_HPP__

#include "vbo.hpp"
#include "glsl.hpp"
#include "main.hpp"

namespace tr
{
  //## Привязка адресов данных вершина к именам параметров
  struct Vertex
  {
    GLfloat *data; // адрес начала данных должен передаваться вызывающей функцией
    struct { GLfloat *x, *y, *z, *w; } position;
    struct { GLfloat *r, *g, *b, *a; } color;
    struct { GLfloat *x, *y, *z, *w; } normal;
    struct { GLfloat *u, *v; } fragment;
    Vertex(GLfloat *data);
  };

  //## Набор данных для формирования в GPU многоугольника с индексацией вершин
  struct snip
  {
    GLsizeiptr idx_offset  = 0; // смещение начала блока индексов в буфере GPU
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
    tr::Vertex vertices[tr::vertices_per_snip] = {&data[0], &data[14], &data[28], &data[42]};

    snip(const tr::f3d &);          // конструктор по-умолчанию
    void relocate(const tr::f3d &); // установка 3D координат фигуры
    GLsizei *reindex(GLsizei);      // настройка индексов для VBO
    void vbo_append(tr::VBO & VBOdata, tr::VBO & VBOidx);
    void vbo_update(tr::VBO &, tr::VBO &, std::pair<GLsizeiptr, GLsizeiptr> &);
  };

  //##  элемент пространства
  class rig
  {
  /* содержит:
   * - индекс типа элемента по которому выбирается текстура и поведение
   * - время установки (будет использоваться) для динамических блоков
   * - если данные записаны в VBO, то смещение адреса данных в GPU */

    public:
      rig(const tr::f3d &);

      short int type = 0;        // тип элемента (текстура, поведение, физика и т.п)
      int time;                  // время создания
      std::forward_list<tr::snip> area {};

    private:
      rig(void) = delete;
      rig operator= (const tr::rig&) = delete;
      rig(const tr::rig&) = delete;
  };

  //## Клас для управления базой данных элементов пространства одного LOD.
  class rigs
  {
    private:
      std::map<tr::f3d, tr::rig> db {};

    public:
      GLuint vert_count = 0; // сумма вершин, переданных в VBO
      float gage = 1.f;      // размер стороны элементов в текущем LOD

      rigs(void){}
      rig* get(float x, float y, float z);
      f3d search_down(float x, float y, float z);
      f3d search_down(const glm::vec3 &);
      size_t size(void) { return db.size(); }
      void emplace(int x, int y, int z);
      bool is_empty(float x, float y, float z);
      bool exist(float x, float y, float z);
  };

} //namespace tr

#endif
