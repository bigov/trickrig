//============================================================================
//
// file: io.hpp
//
//
//
//============================================================================
#ifndef TOOLS_HPP
#define TOOLS_HPP

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

  extern uchar int2uchar(int v); // Вспомогательная функция для структуры "px"

  struct px
  {
    uchar r, g, b, a;
    px(void): r(0x00), g(0x00), b(0x00), a(0x00) {}
    px(uchar vr, uchar vg, uchar vb, uchar va): r(vr), g(vg), b(vb), a(va) {}
    px(int R, int G, int B, int A):
      r(int2uchar(R)),
      g(int2uchar(G)),
      b(int2uchar(B)),
      a(int2uchar(A)) {}
  };

  extern bool operator== (const px &A, const px &B);

  // Служебный класс для хранения в памяти избражений
  class image
  {
    private:

    protected:
      image(const image&)            = delete;
      image& operator=(const image&) = delete;

      uint width = 0;    // ширина изображения в пикселях
      uint height = 0;   // высота изображения в пикселях

      void load(const std::string &filename);

    public:
      // конструкторы
      image(void)                             = default;
      image(uint new_width, uint new_height);
      image(ulong new_width, ulong new_height, const px& color);
      image(const std::string& filename);
      virtual ~image(void)  = default;

      std::vector<px> Data {};


      // публичные методы
      virtual void resize(uint new_width, uint new_height, px Color = {0x00, 0x00, 0x00, 0x00});

      auto get_width(void) const { return width; }
      auto get_height(void) const { return height; }
      void fill(const px& color);
      uchar* uchar_t(void) const;
      px* px_data(void) const;
      void put(image &dst, ulong x, ulong y) const;
      void paint_over(uint x, uint y, px* src_data, uint src_width, uint src_height);
  };


  // Служебный класс для хранения в памяти текстур
  class texture: public image
  {
    private:
      void resize(uint, uint, px) {}

    protected:
      uint columns = 1;          // число ячеек в строке
      uint rows =    1;          // число строк
      uint cell_width = width;   // ширина ячейки в пикселях
      uint cell_height = height; // высота ячейки в пикселях

    public:
      texture(const std::string &filename, uint new_cols = 1, uint new_rows = 1);

      auto get_cell_width(void) const { return cell_width;  }
      auto get_cell_height(void) const { return cell_height; }
      void put(uint col, uint row, image &dst, ulong dst_x, ulong dst_y) const;
  };

  extern std::unique_ptr<char[]> read_chars_file(const std::string &FileName);
  extern std::unique_ptr<unsigned char[]> load_font_file(const char* font_file_name);
  extern std::vector<int> string2unicode(std::string& Text);

  extern int get_msec(void);   // число миллисекунд от начала суток.
  extern int random_int(void);
  extern std::list<std::string> dirs_list(const std::string &path);
  extern void textstring_place(const texture& FontImg, const std::string& TextString,
                     image& Dst, ulong x, ulong y);

  extern unsigned char side_opposite(unsigned char s);
  extern i3d i3d_near(const i3d& P, uchar side, int side_len);

  // Настройка пиксельных шрифтов
  // ----------------------------
  // "FontMap1" - однобайтовые символы
  extern const std::string FontMap1;
  // "FontMap2" - каждый символ занимает по два байта
  extern const std::string FontMap2;
  extern uint FontMap1_len; // значение будет присвоено в конструкторе класса

  extern uint f_len; // количество символов в текстуре шрифта
  extern texture Font12n; //шрифт 07х12 (норм)
  extern texture Font15n; //шрифт 08х15 (норм)
  extern texture Font18n; //шрифт 10x18 (норм)
  extern texture Font18s; //шрифт 10x18 (тень)
  extern texture Font18l; //шрифт 10x18 (светл)

}

#endif //TOOLS_HPP
