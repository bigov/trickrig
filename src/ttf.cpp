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
  void ttf::init(const std::string &file_name, FT_UInt font_size)
  {
    font = file_name;
    pixel_width = font_size;
    return;
  }

  //## Наложение на картинку текстовой строки
  // TODO контроль границ
  //
  void ttf::write_wstring(image & Image, const std::wstring & Text)
  {
    for (const wchar_t & wChar: Text)
    {
      symbol S = CharsMap[wChar];
      for (uint32_t _y = 0; _y < S.height; ++_y)
      for (uint32_t _x = 0; _x < S.width; ++_x)
      {
        // нормализованное значение прозрачности
        float a = static_cast<float>(S.Data[_y * S.width + _x]) / 255.0f;

        // индекс пикселя на исходной картинке
        size_t i = 4 * ((_height + cur_y + _y - S.base) *
          static_cast<size_t>(Image.w) + cur_x + _x);
        
        Image.Data[i] = static_cast<uint8_t>
          (static_cast<float>(Image.Data[i]) * (1.f - a) + _r * a);
        i += 1;
        Image.Data[i] = static_cast<uint8_t>
          (static_cast<float>(Image.Data[i]) * (1.f - a) + _g * a);
        i += 1;
        Image.Data[i] = static_cast<uint8_t>
          (static_cast<float>(Image.Data[i]) * (1.f - a) + _b * a);
        i += 1;
        Image.Data[i] = static_cast<uint8_t>
          (static_cast<float>(Image.Data[i]) * (1.f - a) + _a * a);
      }
      cur_x += S.width;
      // TODO контроль координат и перенос курсора
    }
    return;
  }

  //## Начальные координаты строки (сверху-слева)
  //
  void ttf::set_cursor(uint32_t x, uint32_t y)
  {
    cur_x = x; cur_y = y;
    return;
  }

  //## Установка цвета текста
  //
  void ttf::set_color(uint8_t r, uint8_t g, uint8_t b, uint8_t a)
  {
    set_color(r, g, b);
    _a = static_cast<float>(a); // прозрачность текста
    return;
  }

  //## Для упрощения вычислений цвета пикселей при наложении текста
  // на изображение используются числа в формате с плавающей запятой
  //
  void ttf::set_color(uint8_t r, uint8_t g, uint8_t b)
  {
    _r = static_cast<float>(r);
    _g = static_cast<float>(g);
    _b = static_cast<float>(b);
    _a = 255.f;
    return;
  }

  //## Загрузка в оперативную память изображений символов шрифта
  //
  void ttf::load_chars(std::wstring in_chars_string)
  {
    FT_Library ftLibrary {};
    FT_Init_FreeType(&ftLibrary);
    FT_Face ftFace {};
    FT_New_Face(ftLibrary, font.c_str(), 0, &ftFace);
    FT_Set_Pixel_Sizes(ftFace, pixel_width, 0);

    for (const wchar_t &charcode: in_chars_string)
    try {CharsMap.at(charcode);} catch(...)
    { 
        if (0 != FT_Load_Char(ftFace, charcode, FT_LOAD_RENDER))
        if (0 != FT_Load_Char(ftFace, L'.', FT_LOAD_RENDER)) continue;

        symbol wchar {};
        FT_GlyphSlot slot = ftFace->glyph;
        auto b = slot->bitmap;
        wchar.base   = static_cast<uint32_t>(slot->bitmap_top);
        wchar.height = b.rows;
        wchar.width = static_cast<uint32_t>(slot->advance.x >> 6 );
        wchar.Data.assign(wchar.width * wchar.height, 0);
        
        for (uint32_t _y = 0; _y < b.rows; ++_y)
        for (uint32_t _x = 0; _x < b.width; ++_x)
          wchar.Data[static_cast<uint32_t>(slot->bitmap_left) +
            _x + _y * wchar.width] = b.buffer[_x +
            _y * static_cast<uint32_t>(b.pitch)];

        _height = std::max(_height, wchar.height + 2); //отступ от Y курсора
        CharsMap[charcode] = std::move(wchar);
    }

    FT_Done_Face(ftFace);
    FT_Done_FreeType(ftLibrary);
    return;
  }

} //namespace
