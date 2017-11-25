//----------------------------------------------------------------------------
//
// file: ttf.cpp
//
// заголовок библиотеки для работы с TTF шрифтами
//
//----------------------------------------------------------------------------
#ifndef __TTF_HPP__
#define __TTF_HPP__

#include "main.hpp"
#include "io.hpp"

namespace tr
{

struct Symbol
{
  uint32_t base   = 0; // расстояние от базовой линии
  uint32_t width  = 0; // ширина символа
  uint32_t height = 0; // высота символа
	std::vector<uint8_t> ico {};
};

class TTF
{
	public:
		TTF() {};
		~TTF() {};
		void load_chars(std::wstring chars_row);
		std::map<wchar_t, Symbol> chars {};
		void init(const std::string &file_name, FT_UInt font_size);
		void write_wstring(pngImg &image, const std::wstring &text);
		void set_color(uint8_t r, uint8_t g, uint8_t b, uint8_t a);
		void set_color(uint8_t r, uint8_t g, uint8_t b);
		void set_cursor(uint32_t x, uint32_t y);

	private:
		uint32_t X = 0, Y = 0, // координаты курсора
		_height = 0; // отступ курсора равен высоте самого высокого символа
		float _r = 20.f, _g = 20.f, _b = 20.f, _a = 255.f;
		std::string font {};
		FT_UInt pixel_width = 0;
};
} // namespace
#endif
