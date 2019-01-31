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
#include "io.hpp"

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

class splice: public std::vector<GLfloat*>
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

  std::array<splice, SIDES_COUNT>      Splice {}; // коорднаты стыка с соседним ригом
  // Положения блоков данных по каждой из сторон в буфере GPU
  GLsizeiptr  offset[SIDES_COUNT] = {0, 0, 0, 0, 0, 0};
  // Видимость сторон по умолчанию включена. Для выключения необходимо сравнить с боксами соседних ригов
  bool visible [SIDES_COUNT] = {true, true, true, true, true, true};

  void init_arrays(void);

  // расчет стыков для каждой из сторон
  void splice_side_xp(void);
  void splice_side_xn(void);
  void splice_side_yp(void);
  void splice_side_yn(void);
  void splice_side_zp(void);
  void splice_side_zn(void);

public:
  // конструктор с генератором вершин и значениями по-умолчанию
  box(f3d Base={0.f, 0.f, 0.f}, float lx=1.f, float ly=1.f, float lz=1.f);
  // Конструктор бокса из массива вершин
  box(const std::array<f3d, 8>&);
  ~box() {}

  void splice_calc(u_char side_id);
  void side_fill_data(u_char side_id, std::array<GLfloat, digits_per_snip>& data);
  bool is_visible(u_char side_id, splice& S);
  void offset_write(u_char side_id, GLsizeiptr n);
  GLsizeiptr offset_read(u_char side_id);
  void offset_replace(GLsizeiptr old_n, GLsizeiptr new_n);
  u_char side_id_by_offset(GLsizeiptr);
};

}
#endif // BOX_HPP
