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

class rig // группа элементов, образующих объект пространства
{
  public:
    rig(const i3d& Or): born(tr::get_msec()), Origin(Or) {} // конструктор

    int born;                             // метка времени создания
    i3d Origin {0, 0, 0};                 // координаты опорной точки
    std::vector<box> Boxes {};            // массив боксов
    bool in_vbo = false;                  // данные помещены в VBO

  private:
    rig(void)                   = delete; // конструктор без параметров
    rig(const rig&)             = delete; // дублирующий конструктор
    rig& operator= (const rig&) = delete; // копирующее присваивание
};


}
#endif
