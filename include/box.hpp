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
#define VERT_PER_SIDE 4
#
#define SPLICE_SIZE 8
#
//#define UCH_MAX 255


struct splice
{
  splice(void);
  bool on = false;
  u_char data[SPLICE_SIZE];
  bool operator!= (splice& Other);
};


struct uch2
{
  u_char
  u = 0, v = 0;
};


///
/// \brief The uch3 struct
/// \details Максимальное значение координаты = 255
///
struct uch3
{
  u_char
  x = 0, y = 0, z = 0;
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
  std::array<uch3, VERT_PER_BOX>   AllCoords   {}; // относительные координаты 8 вершин внутри бокса
  std::array<a_uch4, SIDES_COUNT> CursorCoord  {}; // курсор на координаты

  // AllColors - массив цветов вершин позволяет задавать цвета каждой вершины
  // каждой из сторон отдельно, но инициализируется одним цветом по-умолчанию.
  // После инициализации CursorColor указывает на один цвет для всех вершин.
  std::vector<color>              AllColors;   // цвета вершин
  std::array<a_uch4, SIDES_COUNT> CursorColor; // указатель цветов вершин

  std::vector<normal>             AllNormals   {}; // Направления нормалей вершин
  std::array<a_uch4, SIDES_COUNT> CursorNormal {}; // курсор на нормали каждой вершины

  uch2 tex_id[SIDES_COUNT];                           // индексы текстур сторон
  texture Texture2d[SIDES_COUNT * vertices_per_snip]; // координаты текстуры

  std::array<splice, SIDES_COUNT>  Splice {}; // координаты стыка с соседним ригом

  GLsizeiptr offset[SIDES_COUNT];  // Положения блоков данных по каждой из сторон в буфере GPU
  void init_arrays(void);
  bool visible[SIDES_COUNT];       // Видимость сторон

  // расчет стыков для каждой из сторон
  void splice_side_xp(void);
  void splice_side_xn(void);
  void splice_side_yp(void);
  void splice_side_yn(void);
  void splice_side_zp(void);
  void splice_side_zn(void);

public:
  // конструктор с генератором вершин и значениями по-умолчанию
  box(uch3 Base={0, 0, 0}, uch3 Length={UCHAR_MAX, UCHAR_MAX, UCHAR_MAX}, void* p = nullptr);
  // Конструктор бокса из массива вершин
  box(const std::array<uch3, VERT_PER_BOX>&, void* p = nullptr);
  ~box(void) {}

  void* ParentRig;
  u_char side_id_offset(GLsizeiptr);
  void visible_check(u_char side_id, box&);
  void visible_recheck(u_char side_id, box&);
  splice splice_get(u_char side_id);
  void splice_calc(u_char side_id);
  bool side_fill_data(u_char side_id, std::array<GLfloat, digits_per_snip>& data);
  void offset_write(u_char side_id, GLsizeiptr n);
  GLsizeiptr offset_read(u_char side_id);
  void offset_replace(GLsizeiptr old_n, GLsizeiptr new_n);
  u_char side_id_by_offset(GLsizeiptr);
};

}
#endif // BOX_HPP
