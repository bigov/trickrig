//=============================================================================
//
// file: rigs.hpp
//
// Элементы формирования пространства
//
//=============================================================================
#ifndef __RIGS_HPP__
#define __RIGS_HPP__

#include <iostream>
#include <vector>
#include "cube.hpp"
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

  // Четырехугольник из двух треугольников с индексацией 4-х вершин
  struct Quad
  {
    static const size_t num_vertices = 4;
    static const size_t num_indices = 6;
    static const size_t digits_per_vertex = 14;

    // сформируем слегка выпуклые плитки
    glm::vec3 n = glm::normalize(glm::vec3{0.3f, 0.5f,  0.3f});

    GLfloat data[digits_per_vertex * num_vertices] = {
      0.0f, 0.0f, 0.0f, 1.0f, 0.3f, 0.3f, 0.3f, 1.0f,  n.x, n.y,  n.z, 0.0f, 0.0f,   0.0f,
      0.0f, 0.0f, 1.0f, 1.0f, 0.3f, 0.3f, 0.3f, 1.0f,  n.x, n.y, -n.z, 0.0f, 0.125f, 0.0f,
      1.0f, 0.0f, 1.0f, 1.0f, 0.3f, 0.3f, 0.3f, 1.0f, -n.x, n.y, -n.z, 0.0f, 0.125f, 0.125f,
      1.0f, 0.0f, 0.0f, 1.0f, 0.3f, 0.3f, 0.3f, 1.0f, -n.x, n.y,  n.z, 0.0f, 0.0f,   0.125f,
    };
    GLsizei idx[num_indices] = {0, 1, 2, 2, 3, 0};
    tr::Vertex vertices[num_vertices] = {&data[0], &data[14], &data[28], &data[42]};

    GLsizeiptr data_size = sizeof(data);
    GLsizeiptr idx_size = sizeof(idx);

    void relocate(f3d & point);
    GLsizei *reindex(GLsizei stride);
  };

  //##  элемент пространства
  struct Rig
  {
    // содержит:
    // - индекс типа элемента по которому выбирается текстура и поведение
    // - время установки (будет использоваться) для динамических блоков
    // - если данные записаны в VBO, то смещение адреса данных в GPU

    short int type = 0; // тип элемента (текстура, поведение, физика и т.п)
    int time; // время создания
    GLsizeiptr vbo_offset = 0; // смещение адреса блока в VBO
    tr::Quad area {};

    Rig(): time(get_msec()) {}
    Rig(f3d point, short type);
  };

  //## Клас для управления базой данных элементов пространства одного LOD.
  class Rigs
  {
    private:
      std::map<tr::f3d, tr::Rig> db{};
      bool emplace_complete = false;

    public:
      GLuint vert_count = 0; // сумма вершин, переданных в VBO
      float gage = 1.f;      // размер стороны элементов в текущем LOD

      Rigs(void){}
      Rig* get(float x, float y, float z);
      f3d search_down(float x, float y, float z);
      f3d search_down(const glm::vec3 &);
      size_t size(void) { return db.size(); }
      void emplace(int x, int y, int z);
      void emplace(int x, int y, int z, short type);
      void stop_emplacing(void) { emplace_complete = true; }
      bool is_empty(float x, float y, float z);
      bool exist(float x, float y, float z);
  };

} //namespace tr

#endif
