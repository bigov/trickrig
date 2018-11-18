//============================================================================
//
// file: io.hpp
//
//
//
//============================================================================
#ifndef IO_HPP
#define IO_HPP

#include "main.hpp"
#include "png.h"
#if not defined PNG_SIMPLIFIED_READ_SUPPORTED
  ERR("FAILURE: you must update the \"libpng\".");
#endif

namespace tr
{
  typedef std::vector<unsigned char> vec_uchar;

  ///
  /// \brief Вспомогательная функция для структуры "px"
  ///
  extern UCHAR int_to_uchar(int v);

  struct px
  {
    UCHAR r, g, b, a;
    px(void): r(0x00), g(0x00), b(0x00), a(0x00) {}
    px(UCHAR vr, UCHAR vg, UCHAR vb, UCHAR va): r(vr), g(vg), b(vb), a(va) {}
    px(int R, int G, int B, int A):
      r(int_to_uchar(R)),
      g(int_to_uchar(G)),
      b(int_to_uchar(B)),
      a(int_to_uchar(A)) {};
  };

  // Служебный класс для хранения в памяти текстур
  class img
  {
    private:
      UINT
      _w = 0,    // ширина изображения в пикселях
      _h = 0,    // высота изображения в пикселях
      _c = 1,    // число ячеек в строке
      _r = 1,    // число строк
      _wc = 0,   // ширина ячейки в пикселях
      _hc = 0;   // высота ячейки в пикселях

      img(void)                  = delete;
      img(const img&)            = delete;
      img& operator=(const img&) = delete;

    public:
      std::vector<px> Data;
      const UINT &n_cols;  // число ячеек по горизонтали (колонок)
      const UINT &n_rows;  // число ячеек по вертикали (строк)
      const UINT &w_cell;  // число пикселей в ячейке по горизонтали
      const UINT &h_cell;  // число пикселей в ячейке по вертикали
      const UINT &w_summ;  // ширина (в пикселях)
      const UINT &h_summ;  // высота (в пикселях)

      // конструкторы
      img(UINT width, UINT height);
      img(UINT width, UINT height, const px &pixel);
      img(const std::string &filename, UINT cols = 1, UINT rows = 1);

      // публичные методы
      void resize(UINT width, UINT height);
      UCHAR* uchar(void);
      px* px_data(void);
      void load(const std::string &filename);
      void copy(UINT col, UINT row, img &dst, UINT dst_x, UINT dst_y) const;
  };

  extern void read_chars_file(const std::string &FNname, std::vector<char> &Buffer);
  extern int sh2int(short, short);
  extern void info(const std::string &);
  extern int get_msec(void);   // число миллисекунд от начала суток.
  extern int random_int(void);
  extern short random_short(void);

  // структуры для оперирования с трехмерными координатами

  struct i3d { int x = 0, y = 0, z = 0;};
  extern bool operator< (i3d const& left, i3d const& right);

  struct f3d
  {
    float x = 0.f, y = 0.f, z = 0.f;

    // конструкторы для обеспечения инициализации разными типами данных
    f3d(float x, float y, float z): x(x), y(y), z(z) {}

    f3d(double x, double y, double z): x(static_cast<float>(x)),
                                       y(static_cast<float>(y)),
                                       z(static_cast<float>(z)) {}

    f3d(int x, int y, int z): x(static_cast<float>(x)),
                              y(static_cast<float>(y)),
                              z(static_cast<float>(z)) {}

    f3d(glm::vec3 v): x(v[0]), y(v[1]), z(v[2]) {}
  };

}

#endif //IO_HPP
