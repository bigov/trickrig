#include "gui.hpp"
#include "../assets/fonts/map.hpp"

namespace tr
{

func_with_param_ptr menu_screen::callback_selected_row = nullptr;
uint menu_screen::selected_row_id = 0;

std::string font_dir = "../assets/fonts/";
atlas TextureFont { font_dir + font::texture_file, font::texture_cols, font::texture_rows };

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

/// положение символа в текстурной карте
std::array<unsigned int, 2> map_location(const std::string& Sym)
{
  unsigned int i;
  for(i = 0; i < font::symbols_map.size(); i++) if( font::symbols_map[i].S == Sym ) break;
  return { font::symbols_map[i].u, font::symbols_map[i].v };
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
void textstring_place(const atlas &FontImg, const std::string &OutTextString,
                   image& Dst, ulong x, ulong y)
{
  auto TextString = string2vector(OutTextString);

  #ifndef NDEBUG
  if(x > Dst.get_width() - TextString.size() * FontImg.get_cell_width())
    ERR ("gui::add_text - X overflow");
  if(y > Dst.get_height() - FontImg.get_cell_height())
    ERR ("gui::add_text - Y overflow");
  #endif


  ulong n = 0;
  for(const auto& Symbol: TextString)
  {
    auto L = map_location(Symbol);
    FontImg.put(L[0], L[1], Dst, x + (n++) * FontImg.get_cell_width(), y);
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
atlas::atlas(const std::string& filename, uint new_cols, uint new_rows)
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
void atlas::put(uint C, uint R, image& dst, ulong X, ulong Y) const
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
  TextColor = NewColor;

  const char* fonts_family[FONT_COUNT] = { font_normal.c_str(), font_bold.c_str() };
  wft::font Font { fonts_family[weight] };

  Font.set_size(0, height);
  auto FontBitmap = Font.make_bitmap_text(Text);

  resize(FontBitmap.width, FontBitmap.height, TextColor);

  int i = 0;
  for(auto& pixel: Data) pixel.a = FontBitmap.Bits[i++];
}


///
/// \brief button::button
/// \param new_label
/// \param new_state
///
button::button(const std::string& LabelText, func_ptr new_caller,
               uint new_x, uint new_y, MODE new_mode, uint new_width, uint new_height)
{
  width = new_width;
  height = new_height;
  x = new_x;
  y = new_y;
  mode = new_mode;
  caller = new_caller;
  Label = label(LabelText, label_default_height, FONT_NORMAL, TextDefaultColor);
  state_update(BTN_NORMAL);
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

  switch (mode) {
    case LIST_ENTRY:
      draw_list_entry();
      break;
    case BUTTON:
    default:
      draw_button();
      break;
  }
  // Добавить надпись
  paint_over((width - Label.get_width())/2, (height - Label.get_height())/2, Label);
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
void button::draw_button(void)
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
}


///
/// \brief button::make_list_entry
///
void button::draw_list_entry(void)
{
  switch (state) {
    case BTN_PRESSED:
      fill(LinePressedDefaultBgColor);
      break;
    case BTN_OVER:
      fill(LineOverDefaultBgColor);
      break;
    case BTN_DISABLE:
      fill(LineDisableDefaultBgColor);
      break;
    default:
    case BTN_NORMAL:
      fill(LineNormalDefaultBgColor);
  }
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
  label Text {NewTitle, 16, FONT_BOLD};
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
void menu_screen::button_add(uint x, uint y, const std::string& Label,
                             func_ptr new_caller, BTN_STATE new_state)
{
  button Btn(Label, new_caller, x, y);
  Btn.state_update(new_state);
  paint_over(x, y, Btn);
  MenuItems.push_back(Btn);
}


///
/// \brief menu_screen::list_add
/// \param ItemsList
///
void menu_screen::list_add(const std::list<std::string>& ItemsList,
                           func_ptr fn_exit, func_with_param_ptr fn_select,
                           func_ptr fn_add, func_ptr fn_delete)
{
  callback_selected_row = fn_select;
  uint border = 10;
  uint list_width = width - 2 * border;
  uint row_height = label_default_height + 4;

  uint x = border;
  uint y = title_height + border;

  for(auto& Item: ItemsList)
  {
    button Btn(Item, nullptr, x, y, button::LIST_ENTRY, list_width, row_height);
    paint_over(x, y, Btn);
    MenuItems.push_back(Btn);
    y += Btn.get_height() + 1;
  }

  y += border;
  uint bx[4] = {0, 0, 0, 0};
  uint by[4] = {y, y, y, y};
  uint buttons_distance = button_default_height * 0.2;

  // 4 кнопки распределяем по-горизонтали
  if( width > 5*(button_default_width + buttons_distance))
  {
    bx[0] = width/2 - buttons_distance/2 - 2*button_default_width - buttons_distance;
    bx[1] = bx[0] + button_default_width + buttons_distance;
    bx[2] = bx[1] + button_default_width + buttons_distance;
    bx[3] = bx[2] + button_default_width + buttons_distance;
  }
  else // или в два ряда по две
  {
    bx[0] = width/2 - buttons_distance/2 - button_default_width;
    bx[1] = bx[0] + button_default_width + buttons_distance;
    bx[2] = bx[0];
    bx[3] = bx[1];
    by[2] = by[0] + button_default_height + buttons_distance;
    by[3] = by[2];
  }

  button_add(bx[0], by[0], "Отмена",   fn_exit);
  button_add(bx[1], by[1], "Старт",    row_selected);//, BTN_DISABLE);
  button_add(bx[2], by[2], "Добавить", fn_add);
  button_add(bx[3], by[3], "Удалить",  fn_delete, BTN_DISABLE);

}


///
/// \brief menu_screen::row_selected
///
void menu_screen::row_selected(void)
{
  callback_selected_row(selected_row_id);
}


///
/// \brief menu_screen::cursor_event
/// \param x
/// \param y
/// \return
///
bool menu_screen::cursor_event(double x, double y)
{
  bool result = false;

  for(auto& Item: MenuItems)
  {
    if(Item.state_get() == BTN_DISABLE) continue;
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
func_ptr menu_screen::mouse_event(int mouse_button, int action, int)
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


///
/// \brief graphical_user_interface::graphical_user_interface
/// \param OpenGLContext
///
gui::gui(std::shared_ptr<trgl>& OpenGLContext, std::shared_ptr<glm::vec3> CameraLocation)
  : OGLContext(OpenGLContext), ViewFrom(CameraLocation)
{
  OGLContext->get_frame_size(&window_width, &window_height);
  init_vao();
  start_screen();
}


///
/// \brief gui::init_vao
///
void gui::init_vao(void)
{
  glGenVertexArrays(1, &vao_gui);
  glBindVertexArray(vao_gui);
  size_t max_elements_count = 400; // выделяем память VBO на 400 элементов

  vbo VBOindex { GL_ELEMENT_ARRAY_BUFFER }; // Индексный буфер
  // TODO: так как все четырехугольники сторон индексируются одинаково, то
  // можно использовать общий индексный буфер с "vao_3d" из Space3d
  std::vector<GLuint> DataIdx {};
  GLuint s = 0;
  for(size_t i = 0; i < max_elements_count; ++i)
  {
    s = 4*i;
    DataIdx.push_back(0 + s); DataIdx.push_back(1 + s); DataIdx.push_back(2 + s);
    DataIdx.push_back(2 + s); DataIdx.push_back(3 + s); DataIdx.push_back(0 + s);
  }
  VBOindex.allocate(sizeof(GLuint) * DataIdx.size(), DataIdx.data());// индексный VBO

  VBO_xy.allocate  (sizeof(float) * 2 * max_elements_count); // XY VBO
  VBO_uv.allocate  (sizeof(float) * 2 * max_elements_count); // UV VBO
  VBO_rgba.allocate(sizeof(float) * 4 * max_elements_count); // RGBA VBO

  auto A = Program2d->AtribsList.begin();
  VBO_xy.attrib(A->index, A->d_size, A->type, A->normalized, 0, 0);
  ++A;
  VBO_rgba.attrib(A->index, A->d_size, A->type, A->normalized, 0, 0);
  ++A;
  VBO_uv.attrib(A->index, A->d_size, A->type, A->normalized, 0, 0);

  glBindVertexArray(0);
}


///
/// \brief gui::vbo_clear
///
void gui::vbo_clear(void)
{
  indices = 0;
  VBO_xy.clear();
  VBO_uv.clear();
  VBO_rgba.clear();
}


///
/// \brief graphical_user_interface::render
///
void gui::render(void)
{
  glDisable(GL_DEPTH_TEST);
  glEnable(GL_BLEND);
  glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

  RenderBuffer->bind();
  if(!hud_is_enabled) glClear(GL_COLOR_BUFFER_BIT);
  else hud_update();

  glBindVertexArray(vao_gui);
  Program2d->use();
  for(const auto& A: Program2d->AtribsList) glEnableVertexAttribArray(A.index);
  glDrawElements(GL_TRIANGLES, indices, GL_UNSIGNED_INT, nullptr);
  for(const auto& A: Program2d->AtribsList) glDisableVertexAttribArray(A.index);
  Program2d->unuse();

  RenderBuffer->unbind();
  glBindVertexArray(0);
  calc_fps();
}

///
/// \brief gui::mouse_event
/// \param _button
/// \param _action
/// \param _mods
///
void gui::mouse_event(int _button, int _action, int)
{
  if( (_button == MOUSE_BUTTON_LEFT)
  and (_action == RELEASE) )
    std::clog << "mouse event on gui" << std::endl;
}


///
/// \brief graphical_user_interface::keyboard_event
/// \param key
/// \param scancode
/// \param action
/// \param mods
///
void gui::keyboard_event(int key, int, int action, int)
{
  if((key == KEY_ESCAPE) && (action == RELEASE)) return;
}


///
/// \brief gui::data_xy_append
/// \param left
/// \param top
/// \param width
/// \param height
///
std::vector<float> gui::rect_xy(int left, int top, uint width, uint height)
{
  if(left > window_width) left = window_width;
  if(top > window_height) top = window_height;

  float x0 = static_cast<float>(left) * 2.f / static_cast<float>(window_width) - 1.f;
  float y0 = static_cast<float>(top) * 2.f / static_cast<float>(window_height) - 1.f;

  left += width;
  top += height;

  if(left > window_width) left = window_width;
  if(top > window_height) top = window_height;

  float x1 = static_cast<float>(left) * 2.f / static_cast<float>(window_width) - 1.f;
  float y1 = static_cast<float>(top) * 2.f / static_cast<float>(window_height) - 1.f;

  return { x0,y0,  x1,y0,  x1,y1,  x0,y1 };
}


///
/// \brief gui::data_rgba_prepare
/// \param C
///
std::vector<float> gui::rect_rgba(float_color C)
{
  return { C.r,C.g,C.b,C.a, C.r,C.g,C.b,C.a, C.r,C.g,C.b,C.a, C.r,C.g,C.b,C.a};
}


///
/// \brief gui::data_uv_prepare
/// \param u0
/// \param v0
/// \param u1
/// \param v1
///
std::vector<float> gui::rect_uv(const std::string& Sym)
{
    unsigned int i;
    for(i = 0; i < font::symbols_map.size(); i++) if( font::symbols_map[i].S == Sym ) break;

    float u = static_cast<float>(font::symbols_map[i].u);
    float v = static_cast<float>(font::symbols_map[i].v);

    float u0 = font::sym_u_size * u,  v0 = font::sym_v_size * v;
    float u1 = font::sym_u_size + u0, v1 = font::sym_v_size + v0;

    return { u0,v0, u1,v0, u1,v1, u0,v1 };
}


///
/// \brief gui::textrow
/// \param left
/// \param top
/// \param Text
/// \param symbol_width
/// \param height
///
void gui::textrow(uint left, uint top, const std::vector<std::string>& Text,
                  uint symbol_width = 7, uint height = 7 )
{
  float_color BgColor = {0.0f, 0.0f, 0.0f, 0.0f};
  for(const auto& Symbol: Text)
  {
    auto Vxy = rect_xy(left, top, symbol_width, height);
    VBO_xy.append(Vxy.size() * sizeof(float), Vxy.data());

    auto Vrgba = rect_rgba(BgColor);
    VBO_rgba.append(Vrgba.size() * sizeof(float), Vrgba.data());

    auto Vuv = rect_uv(Symbol);
    VBO_uv.append(Vuv.size() * sizeof(float), Vuv.data());

    indices += 6;
    left += symbol_width;
  }
}


///
/// \brief gui::buttom_create
/// \param left
/// \param top
/// \param width
/// \param height
/// \param caller
/// \param Label
///
void gui::rectangle(uint left, uint top, uint width, uint height,
                   float_color rgba)
{
  auto Vxy = rect_xy(left, top, width, height);
  VBO_xy.append(Vxy.size() * sizeof(float), Vxy.data());

  auto Vrgba = rect_rgba(rgba);
  VBO_rgba.append(Vrgba.size() * sizeof(float), Vrgba.data());

  auto Vuv = rect_uv(" ");
  VBO_uv.append(Vuv.size() * sizeof(float), Vuv.data());

  indices += 6;
}


///
/// \brief gui::start_screen
///
void gui::start_screen(void)
{
  unsigned int border = 20;
  rectangle(border, border, window_width - 2*border, window_height - 2*border, {0.9f, 1.f, 0.9f, 1.f});

  // title
  auto Text = string2vector("Добро пожаловать в TrickRig!");
  uint symbol_width = 14;
  uint symbol_height = 21;
  uint title_height = 40;
  rectangle(border, border, window_width - 2*border, title_height, {1.0f, 1.0f, 0.8f, 1.f});
  uint left = border + window_width/2 - Text.size() * symbol_width / 2;
  uint top =  border + title_height/2 - symbol_height/2 + 2;
  textrow(left, top, Text, symbol_width, symbol_height);
}


///
/// \brief space::calc_render_time
///
void gui::calc_fps(void)
{
  static int frames_counter = 0;
  static const std::chrono::seconds one_second(1);

  std::chrono::time_point<sys_clock> t_now = sys_clock::now();
  static auto fps_start = t_now;

  frames_counter++;
  if (t_now - fps_start >= one_second)
  {
    fps_start = t_now;
    FPS = frames_counter;
    frames_counter = 0;
  }
}


///
/// \brief gui::hud
///
void gui::hud_enable(void)
{
  vbo_clear();
  unsigned int height = 60;
  rectangle(0, window_height - height, window_width, height, {0.75f, 1.f, 0.75f, 0.5f});

  // FPS
  uint border = 2;
  auto Text = string2vector("FPS:0000, X:+000.0, Y:+000.0, Z:+000.0");
  uint symbol_width = 7;
  uint symbol_height = 7;
  uint row_height = symbol_height + 6;
  uint row_width = Text.size() * (symbol_width + 1);
  rectangle(border, border, row_width, row_height, {0.8f, 0.8f, 1.0f, 0.5f});
  uint left = border + row_width/2 - Text.size() * symbol_width / 2;
  uint top =  border + row_height/2 - symbol_height/2 + 1;
  textrow(left, top, Text, symbol_width, symbol_height);
  fps_uv_data = (indices/6 - 34) * 8 * sizeof(float); // у 34-х символов обновляемая текстура
  hud_is_enabled = true;
}


///
/// \brief gui::hud_update
///
void gui::hud_update(void)
{
  // счетчик FPS
  char line[35] = {'\0'}; // длина строки с '\0'
  std::sprintf(line, "%.4i, X:%+06.1f, Y:%+06.1f, Z:%+06.1f",
               FPS, ViewFrom->x, ViewFrom->y, ViewFrom->z);
  auto FPSLine = string2vector(line);
  std::vector<float>FPSuv {};
  for(const auto& Symbol: FPSLine)
  {
    auto uv = rect_uv(Symbol);
    FPSuv.insert(FPSuv.end(), uv.begin(), uv.end());
  }

  //std::sprintf(ln, "X:%+06.1f, Y:%+06.1f, Z:%+06.1f, a:%+04.3f, t:%+04.3f",
  ///                ViewFrom->x, ViewFrom->y, ViewFrom->z, look_dir[0], look_dir[1]);

  glBindBuffer(GL_ARRAY_BUFFER, VBO_uv.get_id());
  glBufferSubData(GL_ARRAY_BUFFER, fps_uv_data, FPSuv.size() * sizeof(float), FPSuv.data());
  glBindBuffer(GL_ARRAY_BUFFER, 0);


}

} //namespace tr
