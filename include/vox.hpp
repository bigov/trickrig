/**
        4-----------1
       /| Zp       /|
      / |/        / |
  Yp|/  7--------/--2
    5-----------0  /
    | /         | /
    |/          |/
Xn-(6)----------3-Xp
   /|
 Zn Yn

  0: 1,1,0    Xp: 0,1,2,3
  1: 1,1,1    Yn: 4,5,6,7
  2: 1,0,1    Yp: 4,1,0,5
  3: 1,0,0    Yn: 6,3,2,7
  4: 0,1,1    Zp: 1,4,7,2
  5: 0,1,0    Zn: 5,0,3,6
  6: 0,0,0
  7: 0,0,1

 */

#ifndef VOXEL_HPP
#define VOXEL_HPP

#include "io.hpp"

namespace tr {

#define SIDE_XP       0
#define SIDE_XN       1
#define SIDE_YP       2
#define SIDE_YN       3
#define SIDE_ZP       4
#define SIDE_ZN       5
#define SIDES_COUNT   6

struct uch2
{
  uint8_t u = 0, v = 0;
};


///
/// \brief class vox
/// \details Элементы из которых строится пространство
class vox
{
private:
  GLfloat u_sz = 0.125f;               // размер ячейки текстуры по U
  GLfloat v_sz = 0.125f;               // размер ячейки текстуры по V
  uch2 tex_id[SIDES_COUNT];            // Индексы текстур сторон
  GLsizeiptr vbo_addr[SIDES_COUNT] {}; // Адреса смещения в буфере GPU массивов данных по каждой из сторон
  GLfloat data[digits_per_voxel] {};   // Данные вершин (координаты, цвет, нормали, текстуры)

  std::bitset<6> visibility {0x00};    // Видимость сторон

  void init_data(void);
  void side_color_set(uint8_t side, color C);
  void side_normals_set(uint8_t side);
  void side_texture_set(uint8_t side);
  void side_position_set(uint8_t side);

  vox(void)                   = delete; // конструктор без параметров
  vox(const vox&)             = delete; // дублирующий конструктор
  vox& operator= (const vox&) = delete; // копирующее присваивание

public:
  vox(const i3d&, int);
  vox(std::pair<const i3d&, int> p): vox(p.first, p.second) {}
  ~vox(void) {}

  i3d Origin;       // координаты опорной точки
  int side_len;     // размер стороны
  int born;         // метка времени создания

  void visible_on(uint8_t side_id);
  void visible_off(uint8_t side_id);
  bool is_visible(uint8_t side_id);
  uint8_t get_visibility(void) { return static_cast<u_int8_t>(visibility.to_ulong()); }

  bool in_vbo = false;              // данные помещены в VBO

  uint8_t side_id_by_offset(GLsizeiptr dst);
  bool side_fill_data(uint8_t side_id, GLfloat* data);
  void offset_write(uint8_t side_id, GLsizeiptr n);
  GLsizeiptr offset_read(uint8_t side_id);
  void offset_replace(GLsizeiptr old_n, GLsizeiptr new_n);
};

}
#endif // VOXEL_HPP
