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

#ifndef VOXEL_HPP
#define VOXEL_HPP

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

extern u_char opposite(u_char side_id);

struct uch2
{
  u_char
  u = 0, v = 0;
};


///
/// \brief The int3 struct
/// \details Структура для хранения относительных координат вершины
///
struct int3
{
  int
  x = 0, y = 0, z = 0;
};

class voxel
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

  std::array<int3, VERT_PER_BOX>  AllCoords {};  // относительные координаты 8 вершин вокселя
  std::array<a_uch4, SIDES_COUNT> IdxCoord {};   // курсор на координаты

  // AllColors - массив цветов вершин позволяет задавать цвета каждой вершины
  // каждой из сторон отдельно, но инициализируется одним цветом по-умолчанию.
  // После инициализации CursorColor указывает на один цвет для всех вершин.
  std::vector<color>              AllColors {};  // цвета вершин
  std::array<a_uch4, SIDES_COUNT> IdxColor {};   // указатель цветов вершин

  std::vector<normal>             AllNormals {}; // Направления нормалей вершин
  std::array<a_uch4, SIDES_COUNT> IdxNormal {};  // курсор на нормали каждой вершины

  texture VertTexture[SIDES_COUNT * vertices_per_quad]; // Координаты текстур вершин
  uch2 tex_id[SIDES_COUNT];                             // Индексы текстур сторон

  GLsizeiptr vbo_addr[SIDES_COUNT];  // Адреса смещения в буфере GPU массивов данных по каждой из сторон

  void init_arrays(void);

  voxel(void)                     = delete; // конструктор без параметров
  voxel(const voxel&)             = delete; // дублирующий конструктор
  voxel& operator= (const voxel&) = delete; // копирующее присваивание

public:
  // конструктор с генератором вершин и значениями по-умолчанию
  voxel(const i3d& Or);
  ~voxel(void) {}

  i3d Origin {0, 0, 0};           // координаты опорной точки
  int born;                       // метка времени создания
  bool visible[SIDES_COUNT];      // Видимость сторон
  bool in_vbo = false;            // данные помещены в VBO
  int size = 255;

  u_char side_id_by_offset(GLsizeiptr dst);
  void visible_check(u_char side_id, voxel*);
  void side_visible_calc(u_char side_id, voxel*);
  bool side_fill_data(u_char side_id, GLfloat* data, const f3d& P);
  void offset_write(u_char side_id, GLsizeiptr n);
  GLsizeiptr offset_read(u_char side_id);
  void offset_replace(GLsizeiptr old_n, GLsizeiptr new_n);

};

}
#endif // VOXEL_HPP
