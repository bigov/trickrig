/*
 *
 *
 */

#ifndef RIG_HPP
#define RIG_HPP

#include "io.hpp"
#include "snip.hpp"
#include "box.hpp"

namespace tr
{

// структура для обращения в тексте программы к индексам данных рига
enum RIG_SHIFT_ID {
  SHIFT_X, SHIFT_Y, SHIFT_Z,        // координаты 3D
  SHIFT_YAW, SHIFT_PIT, SHIFT_ROLL, // рыскание (yaw), тангаж (pitch), крен (roll)
  SHIFT_DIGITS                      // число элементов
};


class rig // группа элементов, образующих объект пространства
{
  public:
    int born;                          // метка времени создания
    i3d Origin {0, 0, 0};              // координаты опорной точки
    float shift[SHIFT_DIGITS] = {
      0.f, 0.f, 0.f,                   // сдвиг относительно опорной точки
      0.f, 0.f, 0.f,                   // поворот по трем осям
    };
    char zoom = 1;                     // если zoom < 0, то делим, иначе умножаем на zoom
    std::vector<box> Boxes {};         // массив боксов
    bool in_vbo = false;               // данные помещены в VBO

    rig(void): born(tr::get_msec()) {}                      // конструктор по-умолчанию
    rig(const i3d& Or): born(tr::get_msec()), Origin(Or) {} // конструктор с указанием Origin
    rig(const rig &);                                       // дублирующий конструктор
    rig& operator= (const rig &);                           // копирующее присваивание

    f3d vector(void); // координаты вектора для расчета окончательных координат вершины

  private:
    void copy_data(const rig &);
};


}
#endif
