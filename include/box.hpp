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

extern u_char opposite(u_char side_id);

struct splice
{
  bool on = false;
  u_char data[SPLICE_SIZE] {0, 0, 0, 0, 0, 0, 0, 0};
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

class rig;

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
  std::array<uch3, VERT_PER_BOX>  AllCoords {}; // относительные координаты 8 вершин внутри бокса
  std::array<a_uch4, SIDES_COUNT> IdxCoord {}; // курсор на координаты

  // AllColors - массив цветов вершин позволяет задавать цвета каждой вершины
  // каждой из сторон отдельно, но инициализируется одним цветом по-умолчанию.
  // После инициализации CursorColor указывает на один цвет для всех вершин.
  std::vector<color>              AllColors {}; // цвета вершин
  std::array<a_uch4, SIDES_COUNT> IdxColor {}; // указатель цветов вершин

  std::vector<normal>             AllNormals {}; // Направления нормалей вершин
  std::array<a_uch4, SIDES_COUNT> IdxNormal {}; // курсор на нормали каждой вершины

  uch2 tex_id[SIDES_COUNT]; // Индексы текстур сторон
  // Координаты текстур вершин пересчитываются при смещении вершин
  texture VertTexture[SIDES_COUNT * vertices_per_snip];

  std::array<splice, SIDES_COUNT> Splice {}; // координаты стыка с соседним ригом

  GLsizeiptr vbo_addr[SIDES_COUNT];  // Адреса смещения в буфере GPU массивов данных по каждой из сторон

  void init_arrays(void);
  splice& splice_get(u_char side_id);
  void splice_calc(u_char side_id);
  void texture_calc(u_char side_id);
  void fill_side(u_char side_id);

  // расчет стыков для каждой из сторон
  void splice_side_xp(void);
  void splice_side_xn(void);
  void splice_side_yp(void);
  void splice_side_yn(void);
  void splice_side_zp(void);
  void splice_side_zn(void);

  // заполнение стороны до максимума
  void fill_side_xp(void);
  void fill_side_xn(void);
  void fill_side_yp(void);
  void fill_side_yn(void);
  void fill_side_zp(void);
  void fill_side_zn(void);

  // сдвиг стороны в направлении уменьшения на указанное расстояние
  bool reduce_xp(u_char len);
  bool reduce_xn(u_char len);
  bool reduce_yp(u_char len);
  bool reduce_yn(u_char len);
  bool reduce_zp(u_char len);
  bool reduce_zn(u_char len);

public:
  // конструктор с генератором вершин и значениями по-умолчанию
  box(uch3 Base={0, 0, 0}, uch3 Length={UCHAR_MAX, UCHAR_MAX, UCHAR_MAX}, rig* p=nullptr);
  // Конструктор бокса из массива вершин
  box(const std::array<uch3, VERT_PER_BOX>&, rig* p = nullptr);
  ~box(void) {}

  rig* ParentRig;
  bool visible[SIDES_COUNT];       // Видимость сторон

  u_char side_id_by_offset(GLsizeiptr dst);
  void visible_check(u_char side_id, box&);
  void side_visible_calc(u_char side_id, box&);
  bool side_fill_data(u_char side_id, GLfloat* data, const f3d& P);
  void offset_write(u_char side_id, GLsizeiptr n);
  GLsizeiptr offset_read(u_char side_id);
  void offset_replace(GLsizeiptr old_n, GLsizeiptr new_n);
  bool move_sub(GLsizeiptr vbo_addr);
  bool side_is_max(u_char side_id);
  void side_fill(u_char side_id);
  bool reduce(u_char side, u_char len);

};

}
#endif // BOX_HPP
