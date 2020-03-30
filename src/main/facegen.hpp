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

///
/// \brief class vox
/// \details Элементы из которых строится пространство
class face_gen
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

  face_gen(void)                        = delete; // конструктор без параметров
  face_gen(const face_gen&)             = delete; // дублирующий конструктор
  face_gen& operator= (const face_gen&) = delete; // копирующее присваивание

public:
  face_gen(const i3d& Point, int side_len, uchar face);
  ~face_gen(void) {}

  face_t Face {};
};

}
#endif // VOX_HPP
