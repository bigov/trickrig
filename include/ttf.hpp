//----------------------------------------------------------------------------
//
// file: ttf.cpp
//
// заголовок библиотеки для работы с TTF шрифтами
//
//----------------------------------------------------------------------------
#ifndef TTF_HPP
#define TTF_HPP

#include "main.hpp"
#include "io.hpp"

namespace tr
{

class symbol
{
  public:
    symbol(){}
    ~symbol() { Data.clear(); }

    uint32_t base   = 0; // расстояние от базовой линии
    uint32_t width  = 0; // ширина символа
    uint32_t height = 0; // высота символа
    std::vector<uint8_t> Data {};
};

class ttf
{
  public:
    ttf(void) {}
    ~ttf(void);

    std::map<wchar_t, symbol> CharsMap {};

    void load_chars(std::wstring);
    void init(const std::string &file_name, FT_UInt font_size);
    void write_wstring(image &, const std::wstring &);
    void set_color(uint8_t r, uint8_t g, uint8_t b, uint8_t a);
    void set_color(uint8_t r, uint8_t g, uint8_t b);
    void set_cursor(uint32_t x, uint32_t y);
    double width(const std::wstring &);
    uint32_t height(void);

  private:
    uint32_t cur_x = 0, cur_y = 0, // координаты курсора
    _height = 0; // отступ курсора равен высоте самого высокого символа
    float _r = 0.f, _g = 0.f, _b = 0.f, _a = 255.f;
    std::string font {};
    FT_UInt pixel_width = 0;
};
} // namespace
#endif
