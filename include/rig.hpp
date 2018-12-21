/*
 *
 *
 */

#ifndef RIG_HPP
#define RIG_HPP

#include "io.hpp"
#include "snip.hpp"

namespace tr
{

// структура для обращения в тексте программы к индексам данных рига
enum RIG_SHIFT_ID {
  SHIFT_X, SHIFT_Y, SHIFT_Z,        // координаты 3D
  SHIFT_YAW, SHIFT_PIT, SHIFT_ROLL, // рыскание (yaw), тангаж (pitch), крен (roll)
  SHIFT_ZOOM,                       // масштабирование
  SHIFT_DIGITS                      // число элементов
};


class rig // группа элементов, образующих объект пространства
{
  public:
    int born = 0;                // метка времени создания
    i3d Origin {0,0,0};               // координаты опорной точки
    float shift[SHIFT_DIGITS] = {
      0.f, 0.f, 0.f, // сдвиг поверхности относительно опорной точки
      0.f, 0.f, 0.f, // поворот по трем осям
      1.0f           // размер (масштабирование)
    };
    std::forward_list<snip> Trick {}; // список поверхностей (снипов)
    bool in_vbo = false;                  // данные помещены в VBO
    // -------------------------------------
    rig(void): born(tr::get_msec()) {}    // конструктор по-умолчанию
    rig(const rig &);                 // дублирующий конструктор
    rig& operator= (const rig &);     // копирующее присваивание
};


}
#endif
