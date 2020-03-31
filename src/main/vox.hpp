#ifndef VOX_HPP
#define VOX_HPP

#include "main.hpp"

namespace tr {

// Массив для работы с данными стороны в бинарном виде.
// В нулевой позиции записывается индекс стороны.
using face_t = std::array<unsigned char, bytes_per_face + 1>;

struct uch2
{
  unsigned char u = 0, v = 0;
};

extern face_t face_gen(const i3d& Point, int side_len, uchar face);

///
/// \brief class vox
/// \details Элементы из которых строится пространство
class vox
{
private:
  float u_sz = 0.125f;            // размер ячейки текстуры по U
  float v_sz = 0.125f;            // размер ячейки текстуры по V
  uch2 tex_id[SIDES_COUNT];       // Индексы текстур сторон
  i3d Origin;                     // координаты опорной точки
  color DefaultColor {1.f, 1.f, 1.f, 1.f};
  int side_len;                   // размер стороны
  float data[digits_per_face] {}; // координаты, цвет, нормали, текстуры одной стороны

  void position_set(uchar face);
  void color_set(color C);
  void normals_set(uchar face);
  void texture_set(uchar face);

  vox(void)                        = delete; // конструктор без параметров
  vox(const vox&)             = delete; // дублирующий конструктор
  vox& operator= (const vox&) = delete; // копирующее присваивание

public:
  vox(const i3d& Point, int side_len, uchar face);
  ~vox(void) {}

  face_t Face {};
};

}
#endif // VOX_HPP
