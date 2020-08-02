#include "gui.hpp"
#include "../assets/fonts/map.hpp"

namespace tr
{

std::string font_dir = "../assets/fonts/";
atlas TextureFont { font_dir + font::texture_file, font::texture_cols, font::texture_rows };

bool gui::open = false;
int gui::window_width  = 0; // ширина окна приложения
int gui::window_height = 0; // высота окна приложения

static std::unique_ptr<vbo> VBO_xy   = nullptr;   // координаты вершин
static std::unique_ptr<vbo> VBO_rgba = nullptr; // цвет вершин
static std::unique_ptr<vbo> VBO_uv   = nullptr;   // текстурные координаты

unsigned int gui::indices = 0; // число индексов в 2Д режиме

static const float_color TitleBgColor  { 1.0f, 1.0f, 0.85f, 1.0f };
static const float_color TitleHemColor { 0.7f, 0.7f, 0.70f, 1.0f };

static const colors BtnBgColor =
  { float_color { 0.89f, 0.89f, 0.89f, 1.0f }, // normal
    float_color { 0.95f, 0.95f, 0.95f, 1.0f }, // over
    float_color { 0.85f, 0.85f, 0.85f, 1.0f }, // pressed
    float_color { 0.85f, 0.85f, 0.85f, 1.0f }  // disabled
  };
static const colors BtnHemColor=
  { float_color { 0.70f, 0.70f, 0.70f, 1.0f }, // normal
    float_color { 0.70f, 0.70f, 0.70f, 1.0f }, // over
    float_color { 0.70f, 0.70f, 0.70f, 1.0f }, // pressed
    float_color { 0.70f, 0.70f, 0.70f, 1.0f }  // disabled
  };
static const colors ListBgColor =
  { float_color { 0.90f, 0.90f, 0.99f, 1.0f }, // normal
    float_color { 0.95f, 0.95f, 0.95f, 1.0f }, // over
    float_color { 1.00f, 1.00f, 1.00f, 1.0f }, // pressed
    float_color { 0.85f, 0.85f, 0.85f, 1.0f }  // disabled
  };
static const colors ListHemColor=
  { float_color { 0.85f, 0.85f, 0.90f, 1.0f }, // normal
    float_color { 0.70f, 0.70f, 0.70f, 1.0f }, // over
    float_color { 0.90f, 0.90f, 0.90f, 1.0f }, // pressed
    float_color { 0.70f, 0.70f, 0.70f, 1.0f }  // disabled
  };

std::vector<gui::element_data> gui::Buttons {};
std::vector<gui::element_data> gui::Rows {};

inline float fl(const unsigned char c) { return static_cast<float>(c)/255.f; }

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
  VBO_xy   = std::make_unique<vbo>(GL_ARRAY_BUFFER);
  VBO_rgba = std::make_unique<vbo>(GL_ARRAY_BUFFER);
  VBO_uv   = std::make_unique<vbo>(GL_ARRAY_BUFFER);

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

  VBO_xy->allocate  (sizeof(float) * 2 * max_elements_count); // XY VBO
  VBO_uv->allocate  (sizeof(float) * 2 * max_elements_count); // UV VBO
  VBO_rgba->allocate(sizeof(float) * 4 * max_elements_count); // RGBA VBO

  auto A = Program2d->AtribsList.begin();
  VBO_xy->attrib(A->index, A->d_size, A->type, A->normalized, 0, 0);
  ++A;
  VBO_rgba->attrib(A->index, A->d_size, A->type, A->normalized, 0, 0);
  ++A;
  VBO_uv->attrib(A->index, A->d_size, A->type, A->normalized, 0, 0);

  glBindVertexArray(0);
}


///
/// \brief gui::vbo_clear
///
void gui::clear(void)
{
  indices = 0;
  VBO_xy->clear();
  VBO_uv->clear();
  VBO_rgba->clear();
  Buttons.clear();
}


///
/// \brief graphical_user_interface::render
///
void gui::render(void)
{
  calc_fps();
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
}


///
/// \brief gui::cursor_event
/// \param x
/// \param y
///
void gui::cursor_event(double x, double y)
{
  for(auto& B: Buttons)
  {
    if(x > B.x0 && x < B.x1 && y > B.y0 && y < B.y1)
    {
      if(B.state != ST_OVER)
      {
        B.state = ST_OVER;
        auto Vrgba = rect_rgba(BtnBgColor[B.state]);
        VBO_rgba->update(Vrgba.size() * sizeof(float), Vrgba.data(), B.rgba_stride);
      }
    } else
    {
      if(B.state == ST_OVER)
      {
        B.state = ST_NORMAL;
        auto Vrgba = rect_rgba(BtnBgColor[B.state]);
        VBO_rgba->update(Vrgba.size() * sizeof(float), Vrgba.data(), B.rgba_stride);
      }
    }
  }
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
    for(auto& B: Buttons)
    {
      if(nullptr == B.caller) continue;
      if(B.state == ST_OVER) B.caller();
    }
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
                  uint symbol_width = 7, uint height = 7, uint kerning = 0)
{

  float_color BgColor = {0.0f, 0.0f, 0.0f, 0.0f};
  for(const auto& Symbol: Text)
  {
    auto Vxy = rect_xy(left, top, symbol_width, height);
    VBO_xy->append(Vxy.size() * sizeof(float), Vxy.data());

    auto Vrgba = rect_rgba(BgColor);
    VBO_rgba->append(Vrgba.size() * sizeof(float), Vrgba.data());

    auto Vuv = rect_uv(Symbol);
    VBO_uv->append(Vuv.size() * sizeof(float), Vuv.data());

    indices += 6;
    left += symbol_width + kerning;
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
  VBO_xy->append(Vxy.size() * sizeof(float), Vxy.data());

  auto Vrgba = rect_rgba(rgba);
  VBO_rgba->append(Vrgba.size() * sizeof(float), Vrgba.data());

  auto Vuv = rect_uv(" ");
  VBO_uv->append(Vuv.size() * sizeof(float), Vuv.data());

  indices += 6;
}


///
/// \brief gui::title
/// \param Label
///
void gui::title(const std::string& Label)
{
  // Построение 2Д картинки экрана сначала
  // Очистка всех массивов VAO
  clear();
  // Заливка окна фоновым цветом
  rectangle(menu_border, menu_border, window_width - 2 * menu_border,
            window_height - 2 * menu_border, {0.9f, 1.f, 0.9f, 1.f});

  auto Text = string2vector(Label);
  uint symbol_width = 14;
  uint symbol_height = 21;
  rectangle(menu_border, menu_border, window_width - 2*menu_border, title_height+1, TitleHemColor);
  rectangle(menu_border, menu_border, window_width - 2*menu_border, title_height, TitleBgColor);
  uint left = menu_border + window_width/2 - Text.size() * symbol_width / 2;
  uint top =  menu_border + title_height/2 - symbol_height/2 + 2;
  textrow(left, top, Text, symbol_width, symbol_height);
}


///
/// \brief gui::button_allocation
/// \return
///
/// \details Возвращает координаты новой кнопки и перераспределяет
/// по экрану уже существующие кнопки с учетом добавления новой
///
std::pair<uint, uint> gui::button_allocation(void)
{
  uint left = (window_width - btn_width)/2;
  uint top = (window_height - btn_height + title_height)/2;

  if(Buttons.empty()) return {left, top};

  // Если размещается 4-я кнопка, то распределяем их в 2 колонки
  if(Buttons.size() == 3)
  {
    uint vert_move_dist = (btn_height + btn_padding) / 2;
    uint hor_move_dist = (btn_width + btn_padding) / 2;

    // первые две кнопки сдвигаем влево - вниз
    auto B = Buttons.begin();
    button_move(*B, -hor_move_dist, vert_move_dist);
    B++;
    button_move(*B, -hor_move_dist, vert_move_dist);

    // третью вправо - вверх
    B++;
    button_move(*B, hor_move_dist, 0 - vert_move_dist * 3);

    // координаты 4-й кнопки
    return { left + hor_move_dist, top + vert_move_dist };
  }

  // Вертикальный сдвиг кнопок
  uint move_dist = (btn_height + btn_padding) / 2;
  for(auto& B: Buttons)
  {
    button_move(B, 0, -move_dist);
    top += move_dist;
  }

  return {left, top};
}


///
/// \brief gui::button_move
/// \param Button
/// \param x
/// \param y
///
void gui::button_move(element_data& Button, int x, int y)
{
  auto new_left = static_cast<uint>(Button.x0 + x);
  auto new_top = static_cast<uint>(Button.y0 + y);
  auto stride = Button.xy_stride;

  // Сдвиг рамки
  auto Vxy = rect_xy(new_left - 1, new_top - 1, btn_width + 2, btn_height + 2);
  VBO_xy->update(Vxy.size() * sizeof(float), Vxy.data(), stride);

  auto stride_interval = Vxy.size() * sizeof(float);

  // Сдвиг основного фона кнопки
  stride += stride_interval;
  Vxy = rect_xy(new_left, new_top, btn_width, btn_height);
  VBO_xy->update(Vxy.size() * sizeof(float), Vxy.data(), stride);

  Button.x0 = new_left * 1.0;
  Button.y0 = new_top * 1.0;
  Button.x1 = (new_left + btn_width) * 1.0;
  Button.y1 = (new_top + btn_height) * 1.0;

  // Сдвиг надписи, состоящей из label_size прямоугольников
  new_left += btn_width/2 - Button.label_size * (symbol_width + symbol_kerning) / 2;
  new_top += 5;

  for(size_t i = 0; i < Button.label_size; ++i)
  {
    stride += stride_interval;
    Vxy = rect_xy(new_left, new_top, symbol_width, symbol_height);
    VBO_xy->update(Vxy.size() * sizeof(float), Vxy.data(), stride);
    new_left += symbol_width + symbol_kerning;
  }
}


///
/// \brief gui::create_element
/// \param Label
/// \param new_caller
/// \return
///
gui::element_data gui::create_element(layout L, const std::string &Label,
                                      const colors& BgColor, const colors& HemColor,
                                      func_ptr new_caller, STATES state = ST_NORMAL)
{
  element_data Element {};
  Element.caller = new_caller;
  Element.state = state;
  Element.x0 = L.left * 1.0;
  Element.y0 = L.top * 1.0;
  Element.x1 = (L.left + L.width) * 1.0;
  Element.y1 = (L.top + L.height) * 1.0;

  // рамка элемента
  Element.xy_stride = VBO_xy->get_hem();
  rectangle(L.left-1, L.top-1, L.width+2, L.height+2, HemColor[Element.state]);

  // фоновая заливка
  Element.rgba_stride = VBO_rgba->get_hem();
  rectangle(L.left, L.top, L.width, L.height, BgColor[Element.state]);

  auto Text = string2vector(Label);
  Element.label_size = Text.size();

  // надпись
  L.left += L.width/2 - Text.size() * (symbol_width + symbol_kerning) / 2;
  L.top += 1 + (L.height - symbol_height ) / 2;
  textrow(L.left, L.top, Text, symbol_width, symbol_height, symbol_kerning);

  return Element;
}


///
/// \brief gui::button
/// \param Label
///
void gui::button_append(const std::string &Label, func_ptr new_caller = nullptr)
{
  auto XY = button_allocation();
  layout L {btn_width, btn_height, XY.first, XY.second};
  auto Element = create_element(L, Label, BtnBgColor, BtnHemColor, new_caller);
  Buttons.push_back(Element);
}


///
/// \brief gui::list_insert
/// \param String
///
void gui::list_insert(const std::string& String, STATES state = ST_NORMAL)
{
  layout L { };
  L.width = window_width - menu_border * 4;
  L.height = row_height;
  L.left = menu_border * 2;
  L.top = L.left + title_height + Rows.size() * (row_height + 1);
  auto Element = create_element(L, String, ListBgColor, ListHemColor, nullptr, state);
  Rows.push_back(Element);
}


///
/// \brief gui::start_screen
///
void gui::start_screen(void)
{
  title("Добро пожаловать в TrickRig!");
  button_append("НАСТРОИТЬ", config_screen);
  button_append("ВЫБРАТЬ КАРТУ", select_map);
  button_append("ЗАКРЫТЬ", close);
}


///
/// \brief gui::config_screen
///
void gui::config_screen(void)
{
  title("ВЫБОР ПАРАМЕТРОВ");
  button_append("ЗАКРЫТЬ", start_screen);
}


///
/// \brief gui::select_map
///
void gui::select_map(void)
{
  title("ВЫБОР КАРТЫ");

  auto MapsDir = cfg::user_dir();     // список директорий с картами
  std::vector<std::string> Maps {};
  for(auto& it: std::filesystem::directory_iterator(MapsDir))
    if (std::filesystem::is_directory(it))
      list_insert(cfg::map_name(it.path().string()), ST_PRESSED);

  // DEBUG
  list_insert("debug 1");
  list_insert("debug 2");

  button_append("НОВАЯ КАРТА");
  button_append("УДАЛИТЬ КАРТУ");
  button_append("СТАРТ");
  button_append("ЗАКРЫТЬ", start_screen);
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
  clear();
  unsigned int height = 60;
  rectangle(0, window_height - height, window_width, height, {0.0f, 0.5f, 0.0f, 0.25f});

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
  // счетчик FPS и координаты камеры
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

  glBindBuffer(GL_ARRAY_BUFFER, VBO_uv->get_id());
  glBufferSubData(GL_ARRAY_BUFFER, fps_uv_data, FPSuv.size() * sizeof(float), FPSuv.data());
  glBindBuffer(GL_ARRAY_BUFFER, 0);


}

} //namespace tr
