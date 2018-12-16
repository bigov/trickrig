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

  enum CHAR_TYPE {
    SINGLE,
    UTF8_FIRST,
    UTF8_SECOND,
    UTF8_ERR
  };

  extern CHAR_TYPE char_type(char c);
  extern size_t utf8_size(const std::string &Text);
  extern std::string wstring2string(const std::wstring &w);

  extern u_char int_to_uchar(int v); // Вспомогательная функция для структуры "px"

  struct px
  {
    u_char r, g, b, a;
    px(void): r(0x00), g(0x00), b(0x00), a(0x00) {}
    px(u_char vr, u_char vg, u_char vb, u_char va): r(vr), g(vg), b(vb), a(va) {}
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
      u_int
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
      const u_int &n_cols;  // число ячеек по горизонтали (колонок)
      const u_int &n_rows;  // число ячеек по вертикали (строк)
      const u_int &w_cell;  // число пикселей в ячейке по горизонтали
      const u_int &h_cell;  // число пикселей в ячейке по вертикали
      const u_int &w_summ;  // ширина (в пикселях)
      const u_int &h_summ;  // высота (в пикселях)

      // конструкторы
      img(u_long width, u_long height);
      img(u_long width, u_long height, const px &pixel);
      img(const std::string &filename, u_int cols = 1, u_int rows = 1);

      // публичные методы
      void resize(u_int width, u_int height);
      u_char* uchar(void) const;
      px* px_data(void) const;
      void load(const std::string &filename);
      void copy(u_int col, u_int row, img &dst, u_long dst_x, u_long dst_y) const;
  };

  extern void read_chars_file(const std::string &FNname, std::vector<char> &Buffer);
  extern int sh2int(short, short);
  extern void info(const std::string &);
  extern int get_msec(void);   // число миллисекунд от начала суток.
  extern int random_int(void);
  extern short random_short(void);
  extern std::list<std::string> dirs_list(const std::string &path);

  // структуры для оперирования с трехмерными координатами

  struct i3d { int x=0, y=0, z=0; };
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
