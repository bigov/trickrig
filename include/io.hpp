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
      a(int_to_uchar(A)) {}
  };

  extern bool operator== (const px &A, const px &B);

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
      void clear(void);
      void fill(const px&);
      u_char* uchar(void) const;
      px* px_data(void) const;
      void load(const std::string &filename);
      void copy(u_int col, u_int row, img &dst, u_long dst_x, u_long dst_y) const;
  };

  extern std::unique_ptr<char[]> read_chars_file(const std::string &FileName);
  extern void info(const std::string &);
  extern int get_msec(void);   // число миллисекунд от начала суток.
  extern int random_int(void);
  extern std::list<std::string> dirs_list(const std::string &path);
  extern void textstring_place(const img& FontImg, const std::string& TextString,
                     img& Dst, u_long x, u_long y);

  // Настройка пиксельных шрифтов
  // ----------------------------
  // "FontMap1" - однобайтовые символы
  extern const std::string FontMap1;
  // "FontMap2" - каждый символ занимает по два байта
  extern const std::string FontMap2;
  extern u_int FontMap1_len; // значение будет присвоено в конструкторе класса

  extern u_int f_len; // количество символов в текстуре шрифта
  extern img Font12n; //шрифт 07х12 (норм)
  extern img Font15n; //шрифт 08х15 (норм)
  extern img Font18n; //шрифт 10x18 (норм)
  extern img Font18s; //шрифт 10x18 (тень)
  extern img Font18l; //шрифт 10x18 (светл)

}

#endif //IO_HPP
