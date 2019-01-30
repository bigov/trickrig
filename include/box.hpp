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

#define SIDE_XP      0
#define SIDE_XN      1
#define SIDE_YP      2
#define SIDE_YN      3
#define SIDE_ZP      4
#define SIDE_ZN      5
#define SIDES_COUNT  6
#
#define VERT_PER_BOX 8
#

class splice: public std::vector<float*>
{
public:
  bool operator== (splice& Other);
  bool operator!= (splice& Other);
};

class box
{
private:
  GLfloat u_sz = 0.125f; // размер ячейки текстуры по U
  GLfloat v_sz = 0.125f; // размер ячейки текстуры по V

  // AllCoords - массив координат вершин. CursorCoord - массив
  // из 6 массивов (по числу сторон бокса) по 4 индекса (по числу вершин
  // для плоскости снипа) для получения 3d координат из AllCoords.
  // Благодаря этому (24*4) 4-й байтовых (float) координаты хранятся в массиве
  // из (8*4) float, плюс 24 однобайтовых индекса. Кроме того, это позволяет
  // проще изменять форму бокса перемещая меньшее число вершин, чем если-бы
  // они хранились отдельно для каждой стороны.
  std::array<f3d, VERT_PER_BOX>   AllCoords   {}; // координаты 8 вершин
  std::array<a_uch4, SIDES_COUNT> CursorCoord {}; // курсор на координаты

  // AllColors - массив цветов вершин позволяет задавать цвета каждой вершины
  // каждой из сторон отдельно, но инициализируется одним цветом по-умолчанию.
  // После инициализации CursorColor указывает на один цвет для всех вершин.
  std::vector<color>              AllColors    {};  // цвета вершин
  std::array<a_uch4, SIDES_COUNT> CursorColor  {};  // указатель цветов вершин

  std::vector<normal>             AllNormals   {}; // Направления нормалей вершин
  std::array<a_uch4, SIDES_COUNT> CursorNormal {}; // курсор на нормали каждой вершины

  std::vector<texture>            AllTextures  {}; // координаты текстур
  std::array<a_uch4, SIDES_COUNT> CursorTexture{}; // курсор на коорд. текстуры вершин

  void init_arrays(void);

public:
  // конструктор с генератором вершин и значениями по-умолчанию
  box(f3d Base={0.f, 0.f, 0.f}, float lx=1.f, float ly=1.f, float lz=1.f);

  // Конструктор бокса из массива вершин
  box(const std::array<f3d, 8>&);
  ~box() {}

  std::array<splice, SIDES_COUNT> Splice  {};     // коорднаты стыка с соседним ригом

  // Положения блоков данных по каждой из сторон в буфере GPU
  std::array<GLsizeiptr, SIDES_COUNT> offset { NULL, NULL, NULL, NULL, NULL, NULL };

  // Видимость сторон по умолчанию включена. Для выключения необходимо сравнить с боксами соседних ригов
  std::array<bool, SIDES_COUNT> visible {1, 1, 1, 1, 1, 1};

  // расчет стыков для каждой из сторон
  void splice_side_xp(u_char s);
  void splice_side_xn(u_char s);
  void splice_side_yp(u_char s);
  void splice_side_yn(u_char s);
  void splice_side_zp(u_char s);
  void splice_side_zn(u_char s);

  void fill_side_data(u_char, std::array<GLfloat, digits_per_snip>&);
  bool is_visible(u_char, splice&);
};

}
#endif // BOX_HPP
