#include "gui.hpp"

#define STB_TRUETYPE_IMPLEMENTATION
#include <stb/stb_truetype.h>

namespace tr
{

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

//////////////////////////////////// D - R - A - F - T ////////////////////////////////////
inline int i_mul_fi(float f, int i) { return static_cast<int>(f * static_cast<float>(i)); }
inline int i_mul_if(int i, float f) { return i_mul_fi(f, i); }


///
/// \brief load_font
/// \param Buffer
/// \param font_file_name
///
void load_font(std::vector<unsigned char>& Buffer, const char* font_file_name)
{
  // load font file
  std::ifstream file (font_file_name, std::ifstream::binary);
  if (!file) fprintf(stderr, "error: not found font file");
  file.seekg (0, file.end);    // get length of file
  auto length = file.tellg();
  file.seekg (0, file.beg);
  std::vector<char> buffer(static_cast<size_t>(length), '\0');
  file.read (buffer.data(),length); // read data as a block
  if (!file) fprintf(stderr, "error: only %lld could be read", file.gcount());
  file.close();
  Buffer.resize(static_cast<size_t>(length), '\0');
  memmove(Buffer.data(), buffer.data(), Buffer.size());
}


static inline unsigned int UTF2Unicode(const char *txt, unsigned int &i){
    unsigned int a=txt[i++];
    if((a&0x80)==0)return a;
    if((a&0xE0)==0xC0){
        a=(a&0x1F)<<6;
        a|=txt[i++]&0x3F;
    }else if((a&0xF0)==0xE0){
        a=(a&0xF)<<12;
        a|=(txt[i++]&0x3F)<<6;
        a|=txt[i++]&0x3F;
    }else if((a&0xF8)==0xF0){
        a=(a&0x7)<<18;
        a|=(a&0x3F)<<12;
        a|=(txt[i++]&0x3F)<<6;
        a|=txt[i++]&0x3F;
    }
    return a;
}

void string2int(std::string& Text, std::vector<int>& V)
{
  V.clear();
  unsigned int max = Text.size();
  unsigned int i = 0;
  while (i < max) V.push_back(UTF2Unicode(Text.c_str(), i));
}


//////////////////////////////////// D - R - A - F - T ////////////////////////////////////


///
/// \brief label::label
///
label::label(const std::string& new_text)
{
  Text = new_text;

  // Настройка текстуры
  GLint tex_width = 200;
  GLint tex_height = 80;

  stbtt_fontinfo font_info {};
  std::vector<unsigned char> FnBuffer {};

  load_font(FnBuffer, font_file.c_str());

  if (!stbtt_InitFont(&font_info, FnBuffer.data(), 0)) fprintf(stderr, "failed\n");

  int b_w = tex_width;  // bitmap width
  int b_h = tex_height; // bitmap height
  float l_h = 18.f;     // line height

  std::vector<int> VL {};
  string2int(Text, VL);

  int* word = &VL[0];
  size_t w_len = VL.size();


  // create a bitmap for the phrase
  std::vector<unsigned char> bitmap( static_cast<size_t>(b_w * b_h), '\0' );

  // calculate font scaling
  auto scale = stbtt_ScaleForPixelHeight(&font_info, l_h);

  int left_dist = 2;  // отступ слева
  int top_dist = 12;  // отступ сверху

  int x = left_dist;

  int ascent, descent, lineGap;
  stbtt_GetFontVMetrics(&font_info, &ascent, &descent, &lineGap);

  ascent = i_mul_if(ascent, scale);
  descent = i_mul_if(descent, scale);

  for (size_t i = 0; i < w_len; ++i)
  {
   // how wide is this character
   int ax;
   int lsb;
   stbtt_GetCodepointHMetrics(&font_info, word[i], &ax, &lsb);

   /// get bounding box for character (may be offset to account for chars
   // that dip above or below the line
   int c_x1, c_y1, c_x2, c_y2;
   stbtt_GetCodepointBitmapBox(&font_info, word[i], scale, scale, &c_x1,
       &c_y1, &c_x2, &c_y2);

   // compute y (different characters have different heights
   int y = top_dist + ascent + c_y1;

   // render character (stride and offset is important here)
   int byteOffset = x + i_mul_if(lsb, scale) + (y * b_w);
   stbtt_MakeCodepointBitmap(&font_info, bitmap.data() + byteOffset,
       c_x2 - c_x1, c_y2 - c_y1, b_w, scale, scale, word[i]);

   // advance x
   x += i_mul_if(ax, scale);

   // add kerning
   int kern;
   kern = stbtt_GetCodepointKernAdvance(&font_info, word[i], word[i + 1]);
   x += i_mul_if(kern, scale);
  }

  resize(tex_width, tex_height, {0, 100, 0, 0});

  auto max = bitmap.size();
  for(unsigned int i = 0; i < max; ++i)
  {
    auto n = bitmap[i];
    if(n > 0) Data[i].a = n;
  }

}

} //namespace tr
