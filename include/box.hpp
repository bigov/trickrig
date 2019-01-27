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


class splice: public std::vector<float*>
{
public:
  bool operator== (splice& Other);
  bool operator!= (splice& Other);
};


class box
{
private:
  void init_arrays(void);

public:
  box(f3d&, float l=1.0f);
  box(std::vector<f3d>&);
  ~box() {}

  enum SIDES {SIDE_XP, SIDE_XN, SIDE_YP, SIDE_YN, SIDE_ZP, SIDE_ZN, SIDES_COUNT};

  GLfloat u_sz = 0.125f; // размер ячейки текстуры по U
  GLfloat v_sz = 0.125f; // размер ячейки текстуры по V
  float lod = 1.0f;

  std::vector<f3d> Vertex {};                    // массив координат вершин
  std::vector<a_f4> Color {};                    // Цвет вершин бокса.
  std::array<a_f2, 4> Texture {};                // координаты текстуры
  std::array<a_uch4, SIDES_COUNT> Indexes {};    // индексы вершин для построения плоскости стороны
  std::array<a_f3, SIDES_COUNT> Normals {};      // Направление нормалей по сторонам
  std::array<splice, SIDES_COUNT> Splice {};     // коорднаты стыка с соседним ригом
  std::array<GLsizeiptr, SIDES_COUNT> offset {}; // положение блока данных в буфере GPU

  // расчет стыков для каждой из сторон
  void splice_side_xp(SIDES s);
  void splice_side_xn(SIDES s);
  void splice_side_yp(SIDES s);
  void splice_side_yn(SIDES s);
  void splice_side_zp(SIDES s);
  void splice_side_zn(SIDES s);

  bool visible(SIDES s, splice& V1);
};

}
#endif // BOX_HPP
