/*
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
public:
  box(std::vector<ar_f3>&);
  ~box() {}

  struct side {
    v_uch Indexes {};  // индексы вершин для построения плоскости стороны
    ar_f3 Normal  {};  // нормаль к стороне
    ar_f2 Texture {};  // текстура
    splice Splice {};  // коорднаты стыка с соседним ригом
  };

  enum SIDES {SIDE_XP, SIDE_XN, SIDE_YP, SIDE_YN, SIDE_ZP, SIDE_ZN, SIDES_COUNT};

  std::vector<ar_f3> vertex {}; // массив координат вершин
  std::array<side, 6> Sides {
    side{ v_uch{ 2, 1, 5, 5, 6, 2 }, ar_f3{ 1.0f, 0.0f, 0.0f }, ar_f2{0.0f, 0.0f}},
    side{ v_uch{ 0, 3, 7, 7, 4, 0 }, ar_f3{-1.0f, 0.0f, 0.0f }, ar_f2{0.0f, 0.0f}},
    side{ v_uch{ 0, 1, 2, 2, 3, 0 }, ar_f3{ 0.0f, 1.0f, 0.0f }, ar_f2{0.0f, 0.0f}},
    side{ v_uch{ 7, 6, 5, 5, 4, 7 }, ar_f3{ 0.0f,-1.0f, 0.0f }, ar_f2{0.0f, 0.0f}},
    side{ v_uch{ 1, 0, 4, 4, 5, 1 }, ar_f3{ 0.0f, 0.0f, 1.0f }, ar_f2{0.0f, 0.0f}},
    side{ v_uch{ 3, 2, 6, 6, 7, 3 }, ar_f3{ 0.0f, 0.0f,-1.0f }, ar_f2{0.0f, 0.0f}}
  };

  // расчет стыков для каждой из сторон
  void splice_side_xp(void);
  void splice_side_xn(void);
  void splice_side_yp(void);
  void splice_side_yn(void);
  void splice_side_zp(void);
  void splice_side_zn(void);

  bool visible(SIDES s, splice& V1);
};

/*
TODO: расчет стыков (сплайсов) для каждой стороны
*/

}
#endif // BOX_HPP
