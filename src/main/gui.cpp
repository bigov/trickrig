#include "gui.hpp"

#define STB_TRUETYPE_IMPLEMENTATION
#include <stb/stb_truetype.h>

namespace tr
{
  static inline int i_mul_if(int i, float f) { return static_cast<int>(f * static_cast<float>(i)); }

///
/// \brief element::element
/// \param width
/// \param height
///
element::element(unsigned int width, unsigned int height, px Color)
{
  BgColor = Color;
  resize(width, height, BgColor);
}


///
/// \brief element::draw
/// \param new_state
///
void element::draw(STATE new_state)
{
  if(new_state != ST_COUNT) state = new_state;
}


///
/// \brief label::label
///
label::label(const std::string& new_text, unsigned int new_height,
             FONT_WEIGHT weight, px new_color)
{
  height = new_height;
  Text = new_text;
  const char* font[FONT_COUNT] = { font_normal.c_str(), font_bold.c_str() };
  text_color = new_color;

  stbtt_fontinfo font_info {};
  auto FnBuffer = load_font_file(font[weight]);
  if (!stbtt_InitFont(&font_info, FnBuffer.get(), 0)) fprintf(stderr, "failed\n");

  // calculate font scaling
  auto scale = stbtt_ScaleForPixelHeight(&font_info, static_cast<float>(height));

  int x = 0, ascent = 0, descent = 0, lineGap = 0;
  stbtt_GetFontVMetrics(&font_info, &ascent, &descent, &lineGap);

  ascent = i_mul_if(ascent, scale);
  descent = i_mul_if(descent, scale);

  int char_width = 0, kerning = 0;
  int left_bearing = 0;
  auto UnicodeText = string2unicode(Text);
  int offset = 0;
  int c_x1=0, c_y1=0, c_x2=0, c_y2=0; // bounding box for character

  // Calculate label width
  width = 0;
  for (size_t i = 0; i < UnicodeText.size(); ++i)
  {
    stbtt_GetCodepointHMetrics(&font_info, UnicodeText[i], &char_width, &left_bearing);
    kerning = stbtt_GetCodepointKernAdvance(&font_info, UnicodeText[i], UnicodeText[i + 1])
        + letter_space;
    width += i_mul_if(char_width + kerning, scale);
  }

  std::vector<unsigned char> bitmap( static_cast<size_t>(width * height), 0x00 );
  resize(width, height, text_color);

  for (size_t i = 0; i < UnicodeText.size(); ++i)
  {
    stbtt_GetCodepointHMetrics(&font_info, UnicodeText[i], &char_width, &left_bearing);
    stbtt_GetCodepointBitmapBox(&font_info, UnicodeText[i], scale, scale, &c_x1, &c_y1, &c_x2, &c_y2);

    offset = x + i_mul_if(left_bearing, scale) + (ascent + c_y1) * width;
    stbtt_MakeCodepointBitmap(&font_info, bitmap.data() + offset,
      c_x2 - c_x1, c_y2 - c_y1, width, scale, scale, UnicodeText[i]);

    kerning = stbtt_GetCodepointKernAdvance(&font_info, UnicodeText[i], UnicodeText[i + 1])
        + letter_space;
    x += i_mul_if(char_width + kerning, scale);
  }

  int i = 0;
  for(auto& pixel: Data) pixel.a = bitmap[i++];

}

} //namespace tr
