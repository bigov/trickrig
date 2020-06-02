#include "gui.hpp"

#define STB_TRUETYPE_IMPLEMENTATION
#include <stb/stb_truetype.h>

namespace tr
{

static inline int i_mul_if(int i, float f) { return static_cast<int>(f * static_cast<float>(i)); }

uint f_len = 160; // количество символов в текстуре шрифта
texture Font12n { "../assets/font_07x12_nr.png", f_len }; //шрифт 07х12 (норм)
texture Font15n { "../assets/font_08x15_nr.png", f_len }; //шрифт 08х15 (норм)
texture Font18n { "../assets/font_10x18_nr.png", f_len }; //шрифт 10x18 (норм)
texture Font18s { "../assets/font_10x18_sh.png", f_len }; //шрифт 10x18 (тень)
texture Font18l { "../assets/font_10x18_lt.png", f_len }; //шрифт 10x18 (светл)


// Настройка пиксельных шрифтов
// ----------------------------
// "FontMap1" - однобайтовые символы
const std::string FontMap1 { "_'\"~!?@#$%^&*-+=(){}[]<>\\|/,.:;abcdefghi"
                             "jklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUYWXYZ0"
                             "123456789 "};
// "FontMap2" - каждый символ занимает по два байта
const std::string FontMap2 { "абвгдеёжзийклмнопрстуфхцчшщъыьэюяАБВГДЕЁЖЗ"
                             "ИЙКЛМНОПРСТУФХЦЧШЩЪЫЬЭЮЯ" };
uint FontMap1_len = 0; // значение будет присвоено в конструкторе класса


inline float fl(const unsigned char c) { return static_cast<float>(c)/255.f; }

/*

dst - базовое изображение, scr - накладываемое

out_a = src_a + dst_a * ( 1 - src_a );

1) out_c = (src_c * scr_a + dst_c * dst_a * ( 1 - src_a )) / out_a
 out_a == 0 -> out_c = 0
2) out_c = scr_c + dst_c * ( 1 - src_a );

*/
///
/// \brief blend_1
/// \param src
/// \param dst
/// \return
///
uchar_color blend_1(uchar_color& src, uchar_color& dst)
{
  uint r=0, g=1, b=2, a=3;

  float S[] = { fl(src.r), fl(src.g), fl(src.b), fl(src.a) };
  float D[] = { fl(dst.r), fl(dst.g), fl(dst.b), fl(dst.a) };
  float R[4];

  R[a] = S[a] + D[a] * ( 1.f - S[a] );
  if(R[a] > 0.f)
  {
    R[r] = ( S[r] * S[a] + D[r] * D[a] * ( 1.f - S[a])) / R[a];
    R[g] = ( S[g] * S[a] + D[g] * D[a] * ( 1.f - S[a])) / R[a];
    R[b] = ( S[b] * S[a] + D[b] * D[a] * ( 1.f - S[a])) / R[a];
  } else {
    R[r] = 0.f;
    R[g] = 0.f;
    R[b] = 0.f;
  }
  return { static_cast<uchar>(R[r] * 255),
           static_cast<uchar>(R[g] * 255),
           static_cast<uchar>(R[b] * 255),
           static_cast<uchar>(R[a] * 255)
  };
}


///
/// \brief Добавление текста из текстурного атласа
///
/// \param текстура шрифта
/// \param строка текста
/// \param массив пикселей, в который добавляется текст
/// \param х - координата
/// \param y - координата
///
void textstring_place(const texture &FontImg, const std::string &TextString,
                   image& Dst, ulong x, ulong y)
{
  #ifndef NDEBUG
  if(x > Dst.get_width() - utf8_size(TextString) * FontImg.get_cell_width())
    ERR ("gui::add_text - X overflow");
  if(y > Dst.get_height() - FontImg.get_cell_height())
    ERR ("gui::add_text - Y overflow");
  #endif

  uint row = 0;                        // номер строки в текстуре шрифта
  uint n = 0;                          // номер буквы в выводимой строке
  size_t text_size = TextString.size(); // число байт в строке

  for(size_t i = 0; i < text_size; ++i)
  {
    auto t = char_type(TextString[i]);
    if(t == SINGLE)
    {
      size_t col = FontMap1.find(TextString[i]);
      if(col == std::string::npos) col = 0;
      FontImg.put(col, row, Dst, x + (n++) * FontImg.get_cell_width(), y);
    }
    else if(t == UTF8_FIRST)
    {
      size_t col = FontMap2.find(TextString.substr(i,2));
      if(col == std::string::npos) col = 0;
      else col = FontMap1_len + col/2;
      FontImg.put(col, row, Dst, x + (n++) * FontImg.get_cell_width(), y);
    }
  }
}


///
/// \brief img::img
/// \param width
/// \param height
/// \param pixel
///
image::image(uint new_width, uint new_height, const uchar_color& NewColor)
{
  width = new_width;  // ширина изображения в пикселях
  height = new_height; // высота изображения в пикселях
  Data.resize(width * height, NewColor);
}


///
/// \brief image::image
/// \param filename
///
image::image(const std::string &filename)
{
  load(filename);
}


///
/// \brief Установка размеров
/// \param W
/// \param H
///
void image::resize(uint new_width, uint new_height, const uchar_color& Color)
{
  width = new_width;    // ширина изображения в пикселях
  height = new_height;  // высота изображения в пикселях
  Data.clear();
  Data.resize(width * height, Color);
}


///
/// \brief img::fill
/// \param color
///
void image::fill(const uchar_color& new_color)
{
  Data.clear();
  Data.resize(width * height, new_color);
}


///
/// \brief img::uchar_data
/// \return
///
uchar* image::uchar_t(void) const
{
  return reinterpret_cast<uchar*>(color_data());
}


///
/// \brief img::px_data
/// \return
///
uchar_color* image::color_data(void) const
{
  return const_cast<uchar_color*>(Data.data());
}


///
/// \brief Загрузки избражения из .PNG файла
///
/// \param filename
///
void image::load(const std::string &fname)
{
  png_image info;
  memset(&info, 0, sizeof info);

  info.version = PNG_IMAGE_VERSION;

  if (!png_image_begin_read_from_file(&info, fname.c_str())) ERR("Can't read PNG image file");
  info.format = PNG_FORMAT_RGBA;

  width = info.width;    // ширина изображения в пикселях
  height = info.height;  // высота изображения в пикселях
  Data.resize(width * height, {0x00, 0x00, 0x00, 0x00});

  if (!png_image_finish_read(&info, nullptr, uchar_t(), 0, nullptr ))
  {
    png_image_free(&info);
    ERR(info.message);
  }
}


///
/// \brief     Копирование одного изображения в другое
///
/// \param dst изображение-приемник
/// \param X   координата пикселя приемника
/// \param Y   координата пикселя приемника
///
void image::put(image& dst, ulong x, ulong y) const
{
  uint src_width = width;
  if( dst.width < width + x ) src_width = dst.width - x;
  uint src_height = height;
  if( dst.height < height + y ) src_height = dst.height - y;

  uint i_max = src_height * src_width;// число копируемых пикселей
  uint src_i = 0;                 // индекс начала фрагмента
  uint dst_i = x + y * dst.width; // индекс начала в приемнике
  uint i = 0;                     // сумма скопированных пикселей

  while(i < i_max)
  {
    uint s_row = src_i;           // текущий индекс источника
    uint d_row = dst_i;           // текущий индекс приемника
    uint d_max = d_row + src_width; // конец копируемой строки пикселей

    // В данной версии копируются только полностью непрозрачные пиксели
    while(d_row < d_max)
    {
      if(Data[s_row].a == 0xFF) dst.Data[d_row] = Data[s_row];
      ++d_row;
      ++s_row;
      ++i;
    }

    dst_i += dst.width; // переход на следующую строку приемника
    src_i += width;     // переход на следующую строку источника
  }
}


///
/// \brief image::paint_over
/// \param x
/// \param y
/// \param src_data
/// \param src_width
/// \param src_height
///
void image::paint_over(uint x, uint y, const image& Src)
{
  uchar_color* src_data = Src.color_data();
  uint src_width = Src.get_width();
  uint src_height = Src.get_height();

  auto original_width = src_width;
  if(src_width + x  > width  ) src_width  = width -  x;
  if(src_height + y > height ) src_height = height - y;

  uint i = 0;                          // число скопированных пикселей
  uint i_max = src_height * src_width; // сумма пикселей источника, которые надо скопировать
  uint src_row_start = 0;              // индекс в начале строки источника
  uint dst_row_start = x + y * width;  // индекс начального пикселя приемника

  while(i < i_max)
  {
    uint row_n = src_width;
    uint dst = dst_row_start;
    uint src = src_row_start;
    while(row_n > 0)
    {
      Data[dst] = blend_1( *(src_data + src), Data[dst]);
      dst += 1;
      src += 1;
      row_n -= 1;
      i += 1;
    }
    src_row_start += original_width; // переход на начало следующей строки источника
    dst_row_start += width;          // переход на начало следующей строки приемника
  }
}

///
/// \brief Конструктор c загрузкой данных из файла
/// \param filename
/// \param cols
/// \param rows
///
texture::texture(const std::string& filename, uint new_cols, uint new_rows)
{
  columns = new_cols;
  rows = new_rows;

  load(filename);

  cell_width = width/columns; // ширина ячейки в пикселях
  cell_height = height/rows; // высота ячейки в пикселях
}


///
/// \brief     Копирование указанного фрагмента текстуры в изображение
///
/// \param C   номер ячейки в строке таблицы текстур
/// \param R   номер строки таблицы текстур
/// \param dst изображение-приемник
/// \param X   координата пикселя приемника
/// \param Y   координата пикселя приемника
///
void texture::put(uint C, uint R, image& dst, ulong X, ulong Y) const
{
  if(C >= columns) C = 0;
  if(R >= rows) R = 0;

  uint frag_w = width / columns;   // ширина фрагмента в пикселях
  uint frag_h = height / rows;   // высота фрагмента в пикселях
  uint frag_sz = frag_h * frag_w;  // число копируемых пикселей

  uint frag_i = C * frag_w + R * frag_h * width; // индекс начала фрагмента

  auto dst_i = X + Y * dst.get_width();       // индекс начала в приемнике
  //UINT dst_max = dst.w * dst.h;     // число пикселей в приемнике

  uint i = 0;              // сумма скопированных пикселей
  while(i < frag_sz)
  {
    auto d = dst_i;        // текущий индекс приемника
    uint s = frag_i;       // текущий индекс источника
    auto l = d + frag_w;   // конец копируемой строки пикселей

    // В данной версии копируются только полностью непрозрачные пиксели
    auto *dstData = dst.color_data();
    while(d < l)
    {
      if(Data[s].a == 0xFF) dstData[d] = Data[s];
      ++d;
      ++s;
      ++i;
    }

    dst_i += dst.get_width(); // переход на следующую строку приемника
    frag_i += width;    // переход на следующую строку источника
  }
}


///
/// \brief label::label
///
label::label(const std::string& new_text, unsigned int new_height,
             FONT_STYLE weight, uchar_color NewColor)
{
  height = new_height;
  Text = new_text;
  const char* font[FONT_COUNT] = { font_normal.c_str(), font_bold.c_str() };
  TextColor = NewColor;

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
  resize(width, height, TextColor);

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


///
/// \brief button::button
/// \param new_label
/// \param new_state
///
button::button(const std::string& LabelText, uint new_x, uint new_y, void(*new_caller)(void))
{
  width = default_width;
  height = default_height;
  x = new_x;
  y = new_y;
  caller = new_caller;
  Label = label(LabelText, default_label_height, FONT_NORMAL, FontColor);
  make_body();
}


///
/// \brief button::state_update
/// \param new_state
/// \return
/// \details Если текущий статус равен new_state - возвращается false,
/// иначе - статус обновляется, кнопка перерисовывается и возвращается true
///
bool button::state_update(BTN_STATE new_state)
{
  assert( (new_state < STATES_COUNT) && "new_state out of range" );
  if(state == new_state) return false;

  state = new_state;
  make_body();
  return true;
}


///
/// \brief Построение картинки корпуса кнопки
///
///  Список цветов сверху-вниз
///
/// верхняя и боковые линии: B6B6B3
/// нижняя линия: 91918C
///
/// неактивной (средняя яркость) кнопки:
///   FAFAFA
///   от E7E7E6 -> 24 градации цвета темнее
/// активной кнопки (светлее):
///   FFFFFF
///   от F6F6F6 -> 24 градации цвета темнее
///
void button::make_body(void)
{
  Data.resize(width * height);
  double step = static_cast<double>(height - 3) / 256.0 * 7.0; // градации цвета

  // Используемые цвета
  uchar_color line_0  { 0xB6, 0xB6, 0xB3, 0xFF }; // верх и боковые
  uchar_color line_f  { 0x91, 0x91, 0x8C, 0xFF }; // нижняя
  uchar_color line_1;                             // блик (вторая линия)
  uchar_color line_bg;                            // фоновый цвет

  // Настройка цветовых значений
  switch (state) {
    case BTN_PRESSED:
      line_bg = { 0xDE, 0xDE, 0xDE, 0xFF };
      line_1  = line_bg;
      step = 0;
      break;
    case BTN_OVER:
      line_1  = { 0xFF, 0xFF, 0xFF, 0xFF };
      line_bg = { 0xF6, 0xF6, 0xF6, 0xFF };
      break;
    case BTN_DISABLE:
      line_bg = { 0xCD, 0xCD, 0xCD, 0xFF };
      line_1  = line_bg;
      step = 0;
      break;
    case BTN_NORMAL:
      line_1  = { 0xFA, 0xFA, 0xFA, 0xFF };
      line_bg = { 0xE7, 0xE7, 0xE6, 0xFF };
      break;
    default:
      break;
  }

  // верхняя линия
  size_t i = 0;
  size_t max = width;
  while(i < max) Data[i++] = line_0;

  // вторая линия
  max += width;
  while(i < max) Data[i++] = line_1;

  // основной фон
  uchar S = 0;       // коэффициент построчного уменьшения яркости
  uint np = 0;       // счетчик значений
  double nr = 0.0;   // счетчик строк

  max += width * (height - 3);
  while (i < max)
  {
    Data[i++] = { static_cast<uchar>(line_bg.r - S),
                  static_cast<uchar>(line_bg.g - S),
                  static_cast<uchar>(line_bg.b - S),
                  static_cast<uchar>(line_bg.a) };
    np++;
    if(np >= width)
    {
      np = 0;
      nr += 1.0;
      S = static_cast<uchar>(nr * step);
    }

  }

  // нижняя линия
  max += width;
  while(i < max)
  {
    Data[i++] = line_f;
  }

  // боковинки
  i = 0;
  while(i < max)
  {
    Data[i++] = line_f;
    i += width - 2;
    Data[i++] = line_f;
  }

  // Добавить надпись
  paint_over((width - Label.get_width())/2, (height - Label.get_height())/2, Label);
}


///
/// \brief menu_screen::buttons_clear
///
void menu_screen::init(uint new_width, uint new_height, const std::string& Title)
{
  MenuItems.clear();
  width = new_width;
  height = new_height;
  resize(width, height, ColorMainBg);
  title_draw(Title);
}


///
/// \brief menu_screen::title_draw
/// \param NewTitle
///
void menu_screen::title_draw(const std::string &NewTitle)
{
  image TitleBox{ width - 4, 24, ColorTitleBg};
  label Text {NewTitle, 24, FONT_BOLD};
  TitleBox.paint_over( (TitleBox.get_width() - Text.get_width()) / 2,
    (TitleBox.get_height() - Text.get_height()) / 2, Text);
  paint_over(2, 2, TitleBox);
  title_height = TitleBox.get_height();
}


///
/// \brief menu_screen::add_button
/// \param x
/// \param y
/// \param Label
///
void menu_screen::button_add(uint x, uint y, const std::string& Label, void(*new_caller)(void))
{
  button Btn(Label, x, y, new_caller);
  paint_over(x, y, Btn);
  MenuItems.push_back(Btn);
}


///
/// \brief menu_screen::list_add
/// \param ItemsList
///
void menu_screen::list_add(const std::list<std::string>& ItemsList)
{
  uint vertical_distance = 8; //pixels
  uint x = width / 2;
  uint y = title_height + vertical_distance;

  for(auto& Item: ItemsList)
  {
    button Btn(Item);
    y += Btn.get_height() + vertical_distance;
    paint_over(x - Btn.get_width()/2, y, Btn);
    MenuItems.push_back(Btn);
  }
}



///
/// \brief menu_screen::cursor_event
/// \param x
/// \param y
/// \return
///
bool menu_screen::cursor_event(double x, double y)
{
  //auto _x = static_cast<uint>(rint(x))
  bool result = false;

  for(auto& Item: MenuItems)
  {
    BTN_STATE state = BTN_NORMAL;

    if((x >= Item.x) && (x < (Item.x + Item.get_width())) &&
       (y >= Item.y) && (y < (Item.y + Item.get_height())))
    {
      state = BTN_OVER;
      if(Item.state_get() == BTN_PRESSED) state = BTN_PRESSED;
    }

    if(Item.state_update(state))
    {
      paint_over(Item.x, Item.y, Item);
      result = true;
    }
  }
  return result;
}


///
/// \brief menu_screen::mouse_event
/// \param button
/// \param action
/// \param mods
/// \return
///
fn_pointer menu_screen::mouse_event(int mouse_button, int action, int)
{
  for(auto& Item: MenuItems)
  {
    auto state = Item.state_get();

    if((mouse_button == MOUSE_BUTTON_LEFT) && (action == PRESS) && (state == BTN_OVER))
    {
      state = BTN_PRESSED;
      if(Item.state_update(state)) paint_over(Item.x, Item.y, Item);
    }

    if((mouse_button == MOUSE_BUTTON_LEFT) && (action == RELEASE) && (state == BTN_PRESSED))
    {
      state = BTN_OVER;
      if(Item.state_update(state))
      {
        paint_over(Item.x, Item.y, Item);
        return Item.caller;
      }
    }

  }
  return nullptr;
}

} //namespace tr
