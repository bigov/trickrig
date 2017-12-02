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
  //## Вычисляет число миллисекунд от начала суток.
  //
  // Функция используется (в планах) для контроля вида динамических блоков,
  // у которых тип элемента (переменная t) изменяется во времени.
  extern int get_msec(void);

  //##  элемент пространства
  struct Rig
  {
    // содержит значения
    // - координат центральной точки, в которой расположен;
    // - углов поворота по трем осям
    // - масштаб/размер
    // - индекс типа элемента по которому выбирается текстура и поведение
    // - время установки (оспользуется для динамических блоков
    //
    float angle[3] = {0.f, 0.f, 0.f}; // угол поворота

    short int type = 0; // тип элемента (текстура, поведение, физика и т.п)
    //unsigned char neighbors = TR_T00; // маска расположения соседних блоков.
                                      // Если значение == 64, то данных нет
    int time; // время создания
    std::list<GLsizeiptr> idx {}; // адреса атрибутов инстансов в VBO_Inst

    Rig(): time(get_msec()) {}
    Rig(short t): type(t), time(get_msec()) {}
    void idx_update(GLsizeiptr idSource, GLsizeiptr idTarget);
  };

  //## Клас для управления базой данных элементов пространства.
  //
  class Rigs
  {
    private:
      std::map<tr::f3d, tr::Rig> db{};
      bool emplace_complete = false;

    public:
      GLuint vert_count = 0; // сумма вершин всей сцены, переданных в буфер
      float gage = 1.f;   // размер/масштаб эементов в данном блоке

      Rigs(void){}
      Rig* get(float x, float y, float z);
      f3d search_down(float x, float y, float z);
      f3d search_down(const glm::vec3&);
      size_t size(void) { return db.size(); }
      void emplace(float x, float y, float z, short t);
      void stop_emplacing(void) { emplace_complete = true; }
      bool is_empty(float x, float y, float z);
      bool exist(float x, float y, float z);
  };

} //namespace tr

#endif
