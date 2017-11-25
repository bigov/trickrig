//-----------------------------------------------------------------------------
//
// file: ttf.cpp
//
// Обертка к библиотеке freetype
//
//-----------------------------------------------------------------------------
#include "ttf.hpp"

namespace tr
{

//## Установка параметров загрузки шрифта
//
void TTF::init(const std::string &file_name, FT_UInt font_size)
{
	font = file_name;
	pixel_width = font_size;
	return;
}

//## Наложение на картинку текстовой строки
// TODO контроль границ
//
void TTF::write_wstring(pngImg &image, const std::wstring &text)
{
	for (const wchar_t & wChar: text)
	{
		Symbol S = chars[wChar];
		for (uint32_t _y = 0; _y < S.height; ++_y)
		for (uint32_t _x = 0; _x < S.width; ++_x)
		{
			// нормализованное значение прозрачности и иконке
			float a = static_cast<float>(S.ico[_y * S.width + _x]) / 255.0f;
			// индекс пикселя на исходной картинке
			size_t i = 4 * ((_height + Y + _y - S.base) * 
				static_cast<size_t>(image.w) + X + _x);
			
			image.img[i] = static_cast<uint8_t>
				(static_cast<float>(image.img[i]) * (1.f - a) + _r * a);
			i += 1;
			image.img[i] = static_cast<uint8_t>
				(static_cast<float>(image.img[i]) * (1.f - a) + _g * a);
			i += 1;
			image.img[i] = static_cast<uint8_t>
				(static_cast<float>(image.img[i]) * (1.f - a) + _b * a);
			i += 1;
			image.img[i] = static_cast<uint8_t>
				(static_cast<float>(image.img[i]) * (1.f - a) + _a * a);
		}
		X += S.width;
		// TODO контроль координат и перенос курсора
	}
	return;
}

//## Начальные координаты строки (сверху-слева)
//
void TTF::set_cursor(uint32_t x, uint32_t y)
{
	X = x; Y = y;
	return;
}

//## Установка цвета текста
//
void TTF::set_color(uint8_t r, uint8_t g, uint8_t b, uint8_t a)
{
	set_color(r, g, b);
	_a = static_cast<float>(a); // прозрачность текста
	return;
}

//## Для упрощения вычислений цвета пикселей при наложении текста
// на изображение используются числа в формате с плавающей запятой
//
void TTF::set_color(uint8_t r, uint8_t g, uint8_t b)
{
	_r = static_cast<float>(r);
	_g = static_cast<float>(g);
	_b = static_cast<float>(b);
	_a = 255.f;
	return;
}

//## Загрузка в оперативную память изображений символов шрифта
//
void TTF::load_chars(std::wstring in_chars_string)
{
	FT_Library ftLibrary = 0;
	FT_Init_FreeType(&ftLibrary);
	FT_Face ftFace = 0;
	FT_New_Face(ftLibrary, font.c_str(), 0, &ftFace);
	FT_Set_Pixel_Sizes(ftFace, pixel_width, 0);

	for (const wchar_t &charcode: in_chars_string)
	try {chars.at(charcode);} catch(...)
	{ 
     	if (0 != FT_Load_Char(ftFace, charcode, FT_LOAD_RENDER))
    	if (0 != FT_Load_Char(ftFace, L'.', FT_LOAD_RENDER)) continue;
			
			Symbol wchar {};
			FT_GlyphSlot slot = ftFace->glyph;
			auto b = slot->bitmap;
			wchar.base   = static_cast<uint32_t>(slot->bitmap_top);
			wchar.height = b.rows;
			wchar.width = static_cast<uint32_t>(slot->advance.x >> 6 );
			wchar.ico.assign(wchar.width * wchar.height, 0);
			
			for (uint32_t _y = 0; _y < b.rows; ++_y)
			for (uint32_t _x = 0; _x < b.width; ++_x)
   			wchar.ico[static_cast<uint32_t>(slot->bitmap_left) +
					_x + _y * wchar.width] = b.buffer[_x +
					_y * static_cast<uint32_t>(b.pitch)];

			_height = std::max(_height, wchar.height + 2); //отступ от Y курсора
			chars[charcode] = std::move(wchar);
	}

	FT_Done_Face(ftFace);
  FT_Done_FreeType(ftLibrary);
	return;
}

} //namespace
