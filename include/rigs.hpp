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
  struct Vertex
  {
    struct { GLfloat x = 0.0f, y = 0.0f, z = 0.0f, w = 1.0f; } position;
    struct { GLfloat r = 0.3f, g = 0.3f, b = 0.3f, a = 1.0f; } color;
    struct { GLfloat x = 0.0f, y = 1.0f, z = 0.0f, w = 0.0f; } normal;
    struct { GLfloat u = 0.0f, v = 0.0f; } fragment;
  };

  struct Quad
  {
    tr::Vertex vertices[4];
    GLsizei indices[6] = {0, 1, 2, 2, 3, 0};
    Quad(void);
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
    Rig(short t): type(t), time(get_msec()) {}
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
      void emplace(float x, float y, float z, short t);
      void stop_emplacing(void) { emplace_complete = true; }
      bool is_empty(float x, float y, float z);
      bool exist(float x, float y, float z);
  };

} //namespace tr

#endif
