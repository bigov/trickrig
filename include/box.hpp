/*
 yz
 xyz
 xy
 y
 z
 xz
 x
 0
       0-----------1
      /|          /|
     / 4---------/-5
    / /         / /
   3-----------2 /
   |/          |/  Yp  Zp
   7-----------6    | /
                    |/___ Xp
 */
#ifndef BOX_HPP
#define BOX_HPP

#include "main.hpp"

namespace tr {

enum SIDES {SIDE_XP, SIDE_XN, SIDE_YP, SIDE_YN, SIDE_ZP, SIDE_ZN, SIDES_COUNT};

class splice: public std::vector<float*>
{
public:
  bool operator== (splice& Other);
  bool operator!= (splice& Other);
};

struct color {
    float r = 0.0f;
    float g = 0.0f;
    float b = 0.0f;
    float a = 0.0f;
};

using side_color = std::array<color, vertices_per_snip>;

class box
{
private:
  void init_arrays(void);

public:

  // конструктор с генератором вершин и значениями по-умолчанию
  box(f3d Base={0.f, 0.f, 0.f}, float lx=1.f, float ly=1.f, float lz=1.f);

  // Конструктор бокса из массива вершин
  box(const std::array<f3d, 8>&);
  ~box() {}

  GLfloat u_sz = 0.125f; // размер ячейки текстуры по U
  GLfloat v_sz = 0.125f; // размер ячейки текстуры по V

  std::vector<f3d>                Vertex  {}; // массив координат вершин
  std::vector<side_color> _SC {};             // цвета вершин одной стороны

  std::array<const side_color&, SIDES_COUNT>  Colors {_SC.front{}};

  std::array<a_f2, 4>             Texture {}; // координаты текстуры
  std::array<a_uch4, SIDES_COUNT> SideIdx {}; // индексы вершин для построения плоскости стороны
  std::array<a_f3, SIDES_COUNT>   Normals {}; // Направление нормалей по сторонам
  std::array<splice, SIDES_COUNT> Splice  {}; // коорднаты стыка с соседним ригом

  // Положение блока данных в буфере GPU
  std::array<GLsizeiptr, SIDES_COUNT> offset {NULL,NULL,NULL,NULL,NULL,NULL};

  // Видимость сторон по умолчанию включена. Для выключения необходимо сравнить с боксами соседних ригов
  std::array<bool, SIDES_COUNT> visible {1, 1, 1, 1, 1, 1};

  // расчет стыков для каждой из сторон
  void splice_side_xp(SIDES s);
  void splice_side_xn(SIDES s);
  void splice_side_yp(SIDES s);
  void splice_side_yn(SIDES s);
  void splice_side_zp(SIDES s);
  void splice_side_zn(SIDES s);

  void side_data(SIDES, std::array<GLfloat, digits_per_snip>&);
  bool is_visible(SIDES, splice&);
};

}
#endif // BOX_HPP
