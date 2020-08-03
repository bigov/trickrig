#include "gui.hpp"
#include "../assets/fonts/map.hpp"

namespace tr
{

std::string font_dir = "../assets/fonts/";
atlas TextureFont { font_dir + font::texture_file, font::texture_cols, font::texture_rows };

bool gui::open = false;
bool gui::RUN_3D = false;
bool gui::hud_is_enabled = false;
GLuint gui::vao2d  = 0;
GLsizei gui::fps_uv_data = 0;           // смещение данных FPS в буфере UV

int gui::window_width  = 0; // ширина окна приложения
int gui::window_height = 0; // высота окна приложения
std::unique_ptr<space_3d> gui::Space3d = nullptr;
std::shared_ptr<trgl> gui::OGLContext = nullptr;
std::unique_ptr<glsl> gui::ShowFrameBuf = nullptr;  // Вывод текстуры фреймбуфера на окно
glm::vec3 gui::Cursor3D = { 200.f, 200.f, 2.f }; // положение и размер прицела

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


/// положение символа в текстурной карте
std::array<unsigned int, 2> map_location(const std::string& Sym)
{
  unsigned int i;
  for(i = 0; i < font::symbols_map.size(); i++) if( font::symbols_map[i].S == Sym ) break;
  return { font::symbols_map[i].u, font::symbols_map[i].v };
}


///
/// \brief gui::mode_3d
///
void gui::mode_3d(void)
{
  RUN_3D = true;
  ShowFrameBuf->set_uniform("Cursor", Cursor3D);
}


///
/// \brief gui::mode_2d
///
void gui::mode_2d(void)
{
  RUN_3D = false;
  ShowFrameBuf->set_uniform("Cursor", {0.f, 0.f, 0.f});
}


///
/// \brief graphical_user_interface::graphical_user_interface
/// \param OpenGLContext
///
gui::gui(std::shared_ptr<trgl>& Context)
{
  OGLContext = Context;
  Space3d = std::make_unique<space_3d>(OGLContext);
  ViewFrom = Space3d->ViewFrom;
  OGLContext->get_frame_size(&window_width, &window_height);
  Cursor3D.x = static_cast<float>(window_width/2);
  Cursor3D.y = static_cast<float>(window_height/2);
  init_vao();
  fbuf_program_init();
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
  if(RUN_3D) Space3d->render(); // рендер 3D сцены

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

  /// Кадр сцены рендерится в изображение на (2D) "холсте" фреймбуфера,
  /// или текстуре интерфейса меню. После этого изображение в виде
  /// текстуры накладывается на прямоугольник окна приложения.
  ShowFrameBuf->use();
  vbo_mtx.lock();
  glBindVertexArray(vao2d);
  glDrawArrays(GL_TRIANGLES, 0, 6);
  glBindVertexArray(0);
  OGLContext->swap_buffers();
  vbo_mtx.unlock();
  ShowFrameBuf->unuse();

#ifndef NDEBUG
  CHECK_OPENGL_ERRORS
#endif
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

  for(auto& B: Rows)
  {
    if (B.state == ST_PRESSED) continue;

    if(x > B.x0 && x < B.x1 && y > B.y0 && y < B.y1)
    {
      if(B.state != ST_OVER)
      {
        B.state = ST_OVER;
        auto Vrgba = rect_rgba(ListBgColor[B.state]);
        VBO_rgba->update(Vrgba.size() * sizeof(float), Vrgba.data(), B.rgba_stride);
      }
    } else
    {
      if(B.state == ST_OVER)
      {
        B.state = ST_NORMAL;
        auto Vrgba = rect_rgba(ListBgColor[B.state]);
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
  {
    for(auto& B: Buttons)
    {
      if(nullptr == B.caller) continue;
      if(B.state == ST_OVER) B.caller();
    }

    for(auto& B: Rows)
    {
      if(B.state == ST_OVER)
      {
        // сбросить все в исходное
        for(auto& T: Rows)
        {
          T.state = ST_NORMAL;
          auto Vrgba = rect_rgba(ListBgColor[T.state]);
          VBO_rgba->update(Vrgba.size() * sizeof(float), Vrgba.data(), T.rgba_stride);
        }

        // включить текущую
        B.state = ST_PRESSED;
        auto Vrgba = rect_rgba(ListBgColor[B.state]);
        VBO_rgba->update(Vrgba.size() * sizeof(float), Vrgba.data(), B.rgba_stride);
      }
      if(nullptr != B.caller)  B.caller();
    }

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
/// \brief gui::focus_event
/// \details Потеря окном фокуса в режиме рендера 3D сцены
/// переводит GUI в режим отображения меню
///
void gui::focus_lost_event(void)
{
  if (RUN_3D)
  {
     cfg::map_view_save(Space3d->ViewFrom, Space3d->look_dir);
     //mode_2d();
     OGLContext->cursor_restore();            // Включить указатель мыши
     OGLContext->set_cursor_observer(*this);  // переключить обработчик смещения курсора
     OGLContext->set_mbutton_observer(*this);  // обработчик кнопок мыши
  }
}


///
/// \brief gui::resize_event
/// \param width
/// \param height
///
void gui::resize_event(int w, int h)
{
  window_width  = static_cast<uint>(w);
  window_height = static_cast<uint>(h);

  // пересчет позции координат прицела (центр окна)
  Cursor3D.x = static_cast<float>(w/2);
  Cursor3D.y = static_cast<float>(h/2);
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
/// \brief get_map_dirs
/// \return
///
std::vector<std::string> get_map_dirs(void)
{
  std::vector<std::string> Result {};

  for(auto& it: std::filesystem::directory_iterator(cfg::user_dir()))
    if(std::filesystem::is_directory(it))
      Result.push_back(it.path().string());

  return Result;
}

///
/// \brief gui::select_map
///
void gui::select_map(void)
{
  title("ВЫБОР КАРТЫ");
  auto Maps = get_map_dirs();
  for(const auto& Dir: Maps ) list_insert(cfg::map_name(Dir), ST_PRESSED);

  // DEBUG
  list_insert("debug 1");
  list_insert("debug 2");

  button_append("НОВАЯ КАРТА");
  button_append("УДАЛИТЬ КАРТУ");
  button_append("СТАРТ");
  button_append("ЗАКРЫТЬ", start_screen);
}


///
/// \brief app::map_open
///
void gui::map_open(uint map_id)
{
  auto Maps = get_map_dirs();
  cfg::map_view_load(Maps[map_id], Space3d->ViewFrom, Space3d->look_dir);

  Space3d->load();
  hud_enable();
  mode_3d();
}


///
/// \brief gui::close_map
///
void gui::close_map(void)
{
  cfg::map_view_save(Space3d->ViewFrom, Space3d->look_dir);
  mode_2d();
  OGLContext->cursor_restore();             // Включить указатель мыши
  //OGLContext->set_cursor_observer(*this);   // переключить обработчик смещения курсора
  //OGLContext->set_mbutton_observer(*this);  // обработчик кнопок мыши
  //OGLContext->set_keyboard_observer(*this); // и клавиатуры
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


///
/// \brief gui::fbuf_program_init
///
/// Инициализация GLSL программы обработки текстуры фреймбуфера.
///
/// Текстура фрейм-буфера за счет измения порядка следования координат
/// вершин с 1-2-3-4 на 3-4-1-2 перевернута - верх и низ в сцене
/// меняются местами. Благодаря этому, нулевой координатой (0,0) окна
/// становится более привычный верхний-левый угол, и загруженные из файла
/// изображения текстур применяются без дополнительного переворота.
///
void gui::fbuf_program_init(void)
{

GLfloat WinData[] = { // XY координаты вершин, UV координаты текстуры
  -1.f,-1.f, 0.f, 1.f, //3
   1.f,-1.f, 1.f, 1.f, //4
   1.f, 1.f, 1.f, 0.f, //2

   1.f, 1.f, 1.f, 0.f, //2
  -1.f, 1.f, 0.f, 0.f, //1
  -1.f,-1.f, 0.f, 1.f, //3
};
int vertex_bytes = sizeof(GLfloat) * 4;

glGenVertexArrays(1, &vao2d);
glBindVertexArray(vao2d);

std::list<std::pair<GLenum, std::string>> Shaders {};
Shaders.push_back({ GL_VERTEX_SHADER, cfg::app_key(SHADER_VERT_SCREEN) });
Shaders.push_back({ GL_FRAGMENT_SHADER, cfg::app_key(SHADER_FRAG_SCREEN) });

ShowFrameBuf = std::make_unique<glsl>(Shaders);
ShowFrameBuf->use();
ShowFrameBuf->AtribsList.push_back(
  { ShowFrameBuf->attrib("position"), 2, GL_FLOAT, GL_TRUE, vertex_bytes, 0 * sizeof(GLfloat) });
ShowFrameBuf->AtribsList.push_back(
  { ShowFrameBuf->attrib("texcoord"), 2, GL_FLOAT, GL_TRUE, vertex_bytes, 2 * sizeof(GLfloat) });
glUniform1i(ShowFrameBuf->uniform("WinTexture"), 1); // GL_TEXTURE1 - фрейм-буфер
ShowFrameBuf->set_uniform("Cursor", {0.f, 0.f, 0.f});
ShowFrameBuf->unuse();

vbo VboWin { GL_ARRAY_BUFFER };
VboWin.allocate( sizeof(WinData), WinData );
VboWin.set_attributes(ShowFrameBuf->AtribsList); // настройка положения атрибутов GLSL программы

glBindVertexArray(0);
}

} //namespace tr
