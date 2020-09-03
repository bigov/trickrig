#include "gui.hpp"
#include "font0.hpp"

namespace tr
{
std::unique_ptr<frame_buffer> RenderBuffer = nullptr; // рендер-буфер окна

std::string font_dir = "assets/textures/";
atlas TextureFont { font_dir + font::texture_file, font::texture_cols, font::texture_rows };
static layout LayoutGui {};                // размеры и положение окна

bool gui::open = false;
std::string gui::map_current {};
bool gui::RUN_3D = false;
GLsizei gui::fps_uv_data = 0;         // смещение данных FPS в буфере UV

std::unique_ptr<space_3d> gui::Space3d = nullptr;
std::shared_ptr<trgl> gui::OGLContext = nullptr;
std::unique_ptr<glsl> gui::ProgramFrBuf = nullptr; // Вывод текстуры фреймбуфера на окно
glm::vec3 gui::Cursor3D = { 200.f, 200.f, 2.f };   // положение и размер прицела

static std::unique_ptr<vbo> VBO_xy   = nullptr;    // координаты вершин
static std::unique_ptr<vbo> VBO_rgba = nullptr;    // цвет вершин
static std::unique_ptr<vbo> VBO_uv   = nullptr;    // текстурные координаты

static unsigned int gui_indices = 0; // число индексов в 2Д режиме
func_ptr gui::current_menu = nullptr;

std::unique_ptr<face> gui::Cursor = nullptr; // Текстовый курсор для пользователя
std::vector<std::unique_ptr<face>> gui::SymbolsBuffer {};     // Строка символов
std::vector<element> gui::Buttons {};
std::vector<element> gui::RowsList {};

/// положение символа в текстурной карте
std::array<unsigned int, 2> map_location(const std::string& Sym)
{
  unsigned int i;
  for(i = 0; i < font::symbols_map.size(); i++) if( font::symbols_map[i].S == Sym ) break;
  return { font::symbols_map[i].u, font::symbols_map[i].v };
}


///
/// \brief rect_xy
/// \param left
/// \param top
/// \param width
/// \param height
///
std::vector<float> rect_xy(const layout& L)
{
  auto left = L.left;
  auto top = L.top;

  if(left > LayoutGui.width) left = LayoutGui.width;
  if(top > LayoutGui.height) top = LayoutGui.height;

  float x0 = static_cast<float>(left) * 2.f / static_cast<float>(LayoutGui.width) - 1.f;
  float y0 = static_cast<float>(top) * 2.f / static_cast<float>(LayoutGui.height) - 1.f;

  left += L.width;
  top += L.height;

  if(left > LayoutGui.width) left = LayoutGui.width;
  if(top > LayoutGui.height) top = LayoutGui.height;

  float x1 = static_cast<float>(left) * 2.f / static_cast<float>(LayoutGui.width) - 1.f;
  float y1 = static_cast<float>(top) * 2.f / static_cast<float>(LayoutGui.height) - 1.f;

  return { x0,y0,  x1,y0,  x1,y1,  x0,y1 };
}


///
/// \brief rect_rgba
/// \param C
///
std::vector<float> rect_rgba(float_color C)
{
  return { C.r,C.g,C.b,C.a, C.r,C.g,C.b,C.a, C.r,C.g,C.b,C.a, C.r,C.g,C.b,C.a};
}


///
/// \brief rect_uv
/// \param u0
/// \param v0
/// \param u1
/// \param v1
///
std::vector<float> rect_uv(const std::string& Sym)
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
/// \brief load_font_texture
/// \details  Загрузка текстуры шрифта
///
void load_textures(void)
{
  glActiveTexture(GL_TEXTURE4);
  GLuint texture_font = 0;
  glGenTextures(1, &texture_font);
  glBindTexture(GL_TEXTURE_2D, texture_font);

  GLint level_of_details = 0;
  GLint frame = 0;

  GLint internalFormat = GL_RED; // Number of color components provided by source image
  GLenum format = GL_RGBA;       // The format, how the image is represented in memory

  glTexImage2D(GL_TEXTURE_2D, level_of_details, internalFormat,
               static_cast<GLsizei>(TextureFont.get_width()),
               static_cast<GLsizei>(TextureFont.get_height()),
               frame, format, GL_UNSIGNED_BYTE, TextureFont.uchar_t());

  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST_MIPMAP_LINEAR);
  glGenerateMipmap(GL_TEXTURE_2D);

  // id тектуры HUD & GUI - меню
  GLuint texture_gui = 0;
  glActiveTexture(GL_TEXTURE2);
  glGenTextures(1, &texture_gui); // id тектуры HUD & GUI - меню
  glBindTexture(GL_TEXTURE_2D, texture_gui);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
}


///
/// \brief char3d::char3d
/// \param left
/// \param top
/// \param Symbol
/// \param symbol_width
/// \param symbol_height
/// \param BgColor
///
face::face(const layout& L, const std::string& Symbol,
                  float_color BgColor)
{
#ifndef NDEBUG
  assert(VBO_xy   != nullptr && "VBO_xy не инициализирован" );
  assert(VBO_rgba != nullptr && "VBO_rgba не инициализирован" );
  assert(VBO_uv   != nullptr && "VBO_uv не инициализирован" );
#endif

  if(Symbol.empty()) return;
  Char = Symbol;
  Layout = L;

  auto Vxy = rect_xy(Layout);
  Addr.xy.offset = VBO_xy->get_hem();
  Addr.xy.size = Vxy.size() * sizeof(float);
  VBO_xy->append(Addr.xy.size, Vxy.data());

  auto Vrgba = rect_rgba(BgColor);
  Addr.rgba.offset = VBO_rgba->get_hem();
  Addr.rgba.size = Vrgba.size() * sizeof(float);
  VBO_rgba->append(Addr.rgba.size, Vrgba.data());

  auto Vuv = rect_uv(Symbol);
  Addr.uv.offset = VBO_uv->get_hem();
  Addr.uv.size = Vuv.size() * sizeof(float);
  VBO_uv->append(Addr.uv.size, Vuv.data());

  gui_indices += 6;
}


///
/// \brief char3d::update
/// \param Symbol
///
void face::update_uv(const std::string& Symbol)
{
  if(Symbol.empty()) return;
  Char = Symbol;
  auto Vuv = rect_uv(Symbol);
  VBO_uv->update(Addr.uv.size, Vuv.data(), Addr.uv.offset);
}


///
/// \brief char3d::update_xy
/// \param Layout
///
void face::update_xy(const layout &L)
{
  Layout = L;
  auto Vxy = rect_xy(Layout);

#ifndef NDEBUG
  assert( Addr.xy.size == static_cast<GLsizeiptr>(Vxy.size() * sizeof(float))
          && "Размер массива не должен меняться.");
#endif

  VBO_xy->update(Addr.xy.size, Vxy.data(), Addr.xy.offset);
}


///
/// \brief face::update_rgba
/// \param Color
///
void face::update_rgba(const float_color &Color)
{
  auto Vrgba = rect_rgba(Color);
  VBO_rgba->update(Addr.rgba.size, Vrgba.data(), Addr.rgba.offset);
}


///
/// \brief face::move_xy
/// \param x
/// \param y
///
void face::move_xy(const uint x, uint y)
{
  layout NewLayout = { Layout.width, Layout.height, Layout.left + x, Layout.top + y };
  update_xy(NewLayout);
}


///
/// \brief char3d::clear
///
void face::clear(void){
  if(Char.empty()) return;
  Char.clear();

  if(Addr.xy.size + Addr.xy.offset <= VBO_xy->get_hem())
    VBO_xy->remove(Addr.xy.size, Addr.xy.offset);
  else
    std::cerr << "\n" << __PRETTY_FUNCTION__ << "\n"
              << "VBO_xy укорочен независимым процессом" << std::endl;

  if(Addr.rgba.size + Addr.rgba.offset <= VBO_rgba->get_hem())
    VBO_rgba->remove(Addr.rgba.size, Addr.rgba.offset);
  else
    std::cerr << "\n" << __PRETTY_FUNCTION__ << "\n"
              << "VBO_rgba укорочен независимым процессом" << std::endl;

  if(Addr.uv.size + Addr.uv.offset <= VBO_uv->get_hem())
    VBO_uv->remove(Addr.uv.size, Addr.uv.offset);
  else
    std::cerr << "\n" << __PRETTY_FUNCTION__ << "\n"
              << "VBO_uv укорочен независимым процессом" << std::endl;

  Addr.xy.size     = 0;
  Addr.xy.offset   = 0;
  Addr.rgba.size   = 0;
  Addr.rgba.offset = 0;
  Addr.uv.size     = 0;
  Addr.uv.offset   = 0;

  if(gui_indices >= 6)
    gui_indices -= 6;
  else
    std::cerr << "\n" << __PRETTY_FUNCTION__ << "\n"
              << "gui_indices укорочен независимым процессом" << std::endl;
}


///
/// \brief char3d::~char3d
///
face::~face(void){
  if(!Char.empty()) clear();
}


///
/// \brief graphical_user_interface::graphical_user_interface
/// \param OpenGLContext
///
gui::gui(void)
{
  std::string title = std::string(APP_NAME) + " v." + std::string(APP_VERSION);
#ifndef NDEBUG
  if(strcmp(USE_CLANG, "FALSE") == 0) title += " [GCC]";
  else title += " [Clang]";
  title += " (debug mode)";
#endif

  OGLContext = std::make_shared<trgl>(title.c_str());
  LayoutGui = cfg::WinLayout;
  OGLContext->set_window(LayoutGui.width, LayoutGui.height, MIN_GUI_WIDTH, MIN_GUI_HEIGHT, LayoutGui.left, LayoutGui.top);

  load_textures();

  // настройка рендер-буфера
  RenderBuffer = std::make_unique<frame_buffer>(LayoutGui.width, LayoutGui.height);
  Space3d = std::make_unique<space_3d>(OGLContext);

  Cursor3D.x = static_cast<float>(LayoutGui.width/2);
  Cursor3D.y = static_cast<float>(LayoutGui.height/2);

  program_2d_init();      // Шейдерная программа для построения 2D элементов пользовательского интерфейса
  program_fbuf_init();

  OGLContext->set_error_observer(*this);     // отслеживание ошибок
  OGLContext->set_cursor_observer(*this);    // курсор мыши в окне
  OGLContext->set_char_observer(*this);      // ввод с клавиатуры
  OGLContext->set_mbutton_observer(*this);   // кнопки мыши
  OGLContext->set_keyboard_observer(*this);  // клавиши клавиатуры
  OGLContext->set_position_observer(*this);  // положение окна
  OGLContext->add_size_observer(*this);      // размер окна
  OGLContext->set_close_observer(*this);     // закрытие окна
  OGLContext->set_focuslost_observer(*this); // потеря окном фокуса ввода

  open = true;
  screen_start();
}



///
/// \brief gui::add_text_cursor
/// \param _Fn       шрифт ввода
/// \param _Dst      строка ввода
/// \param position  номер позиции курсора в строке ввода
/// \details Формирование курсора ввода, моргающего с интервалом в пол-секунды
///
//void app::cursor_text_row(const atlas &_Fn, image &_Dst, size_t position)
void gui::cursor_text_row(const atlas&, image&, size_t)
{
/*
  uchar_color c = {0x11, 0xDD, 0x00, 0xFF};
  auto tm = std::chrono::duration_cast<std::chrono::milliseconds>
      ( std::chrono::system_clock::now()-TimeStart ).count();

  auto tc = trunc(tm/1000) * 1000;
  if(tm - tc > 500) c.a = 0xFF;
  else c.a = 0x00;
*/
//  image Cursor {3, _Fn.get_cell_height(), c};
//  Cursor.put(_Dst, _Fn.get_cell_width() * (position + 1) + 1,
//              (_Dst.get_height() - _Fn.get_cell_height()) / 2 );
}


///
/// \brief gui::remove_map
///
void gui::remove_map(void)
{
  /*
  auto map_dir = Maps[row_selected -1].Folder;
  if(fs::exists(map_dir))
  {
    fs::current_path(cfg::user_dir());
    dirs_list(cfg::user_dir());
    try
    {
      fs::remove_all(map_dir);
    }
    catch(...)
    {
#ifndef NDEBUG
      std::cerr << "Can't remove the map: " + map_dir + "\n";
#endif
    }
  }
  row_selected = 0;

  // обновить список карт
  Maps.clear();
  auto MapsDirs = dirs_list(cfg::user_dir());
  for(auto &P: MapsDirs)
  {
    auto MapName = cfg::map_name(P);
    Maps.push_back(map(P, MapName));
  }
  */
}


///
/// \brief gui::program_fbuf_init
///
/// Инициализация GLSL программы обработки текстуры фреймбуфера.
///
/// Текстура фрейм-буфера за счет измения порядка следования координат
/// вершин с 1-2-3-4 на 3-4-1-2 перевернута - верх и низ в сцене
/// меняются местами. Благодаря этому, нулевой координатой (0,0) окна
/// становится более привычный верхний-левый угол, и загруженные из файла
/// изображения текстур применяются без дополнительного переворота.
///
void gui::program_fbuf_init(void)
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

glGenVertexArrays(1, &vao_fbuf);
glBindVertexArray(vao_fbuf);

std::list<std::pair<GLenum, std::string>> Shaders {};
Shaders.push_back({ GL_VERTEX_SHADER, cfg::app_key(SHADER_VERT_SCREEN) });
Shaders.push_back({ GL_FRAGMENT_SHADER, cfg::app_key(SHADER_FRAG_SCREEN) });

ProgramFrBuf = std::make_unique<glsl>(Shaders);
ProgramFrBuf->use();
ProgramFrBuf->AtribsList.push_back(
  { ProgramFrBuf->attrib("position"), 2, GL_FLOAT, GL_TRUE, vertex_bytes, 0 * sizeof(GLfloat) });
ProgramFrBuf->AtribsList.push_back(
  { ProgramFrBuf->attrib("texcoord"), 2, GL_FLOAT, GL_TRUE, vertex_bytes, 2 * sizeof(GLfloat) });
glUniform1i(ProgramFrBuf->uniform("WinTexture"), 1); // GL_TEXTURE1 - фрейм-буфер
ProgramFrBuf->set_uniform("Cursor", {0.f, 0.f, 0.f});

vbo VboWin { GL_ARRAY_BUFFER };
VboWin.allocate( sizeof(WinData), WinData );
VboWin.set_attributes(ProgramFrBuf->AtribsList); // настройка положения атрибутов GLSL программы

glBindVertexArray(0);
ProgramFrBuf->unuse();
}


///
/// \brief init_prog_2d
///
void gui::program_2d_init(void)
{
  std::list<std::pair<GLenum, std::string>> Shaders {};
  Shaders.push_back({ GL_VERTEX_SHADER, "assets/shaders/2d_vert.glsl" });
  Shaders.push_back({ GL_FRAGMENT_SHADER, "assets/shaders/2d_frag.glsl" });

  Program2d = std::make_unique<glsl>(Shaders);
  Program2d->use();

  // VBO2d_base Обработка массива с данными 2D-координат и цвета вершин
  GLsizei stride = sizeof(GLfloat) * 6;
  Program2d->AtribsList.push_back({Program2d->attrib("vCoordXY"), 2, GL_FLOAT, GL_TRUE, stride, 0 * sizeof(GLfloat)});
  Program2d->AtribsList.push_back({Program2d->attrib("vColor"), 4, GL_FLOAT, GL_TRUE, stride, 2 * sizeof(GLfloat)});

  // VBO2d_uv Массив текстурных координат UV заполняется отдельно. Может меняться динамически.
  Program2d->AtribsList.push_back({Program2d->attrib("vCoordUV"), 2, GL_FLOAT, GL_TRUE, 0, 0});

  glUniform1i(Program2d->uniform("font_texture"), 4);  // glActiveTexture(GL_TEXTURE4)

  VBO_xy   = std::make_unique<vbo>(GL_ARRAY_BUFFER);
  VBO_rgba = std::make_unique<vbo>(GL_ARRAY_BUFFER);
  VBO_uv   = std::make_unique<vbo>(GL_ARRAY_BUFFER);

  glGenVertexArrays(1, &vao_2d);
  glBindVertexArray(vao_2d);
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
  Program2d->unuse();
}


///
/// \brief gui::vbo_clear
///
void gui::clear(void)
{
  gui_indices = 0;
  VBO_xy->clear();
  VBO_uv->clear();
  VBO_rgba->clear();
  Buttons.clear();
  RowsList.clear();
  SymbolsBuffer.clear();
  Cursor = nullptr;
}


///
/// \brief graphical_user_interface::render
///
bool gui::render(void)
{
  if(RUN_3D)
  {
    Space3d->render(); // рендер 3D сцены
    calc_fps();
  }
  else
  {
    callback_render();
  }

  glDisable(GL_DEPTH_TEST);
  glEnable(GL_BLEND);
  glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

  RenderBuffer->bind();
  if(!RUN_3D) glClear(GL_COLOR_BUFFER_BIT);
  else hud_update();

  glBindVertexArray(vao_2d);
  Program2d->use();
  for(const auto& A: Program2d->AtribsList) glEnableVertexAttribArray(A.index);
  glDrawElements(GL_TRIANGLES, gui_indices, GL_UNSIGNED_INT, nullptr);
  for(const auto& A: Program2d->AtribsList) glDisableVertexAttribArray(A.index);
  Program2d->unuse();
  RenderBuffer->unbind();

  /// Кадр сцены рендерится в изображение на (2D) "холсте" фреймбуфера,
  /// или текстуре интерфейса меню. После этого изображение в виде
  /// текстуры накладывается на прямоугольник окна приложения.
  ProgramFrBuf->use();
  vbo_mtx.lock();
  glBindVertexArray(vao_fbuf);
  glDrawArrays(GL_TRIANGLES, 0, 6);
  glBindVertexArray(0);
  OGLContext->swap_buffers();
  vbo_mtx.unlock();
  ProgramFrBuf->unuse();

#ifndef NDEBUG
  CHECK_OPENGL_ERRORS
#endif

  return open;
}


///
/// \brief gui::cursor_event
/// \param x
/// \param y
///
void gui::event_cursor(double x, double y)
{
  if(RUN_3D)
  {
    Space3d->cursor_event(x, y);
    return;
  }

  for(auto& B: Buttons)
  {
    if(x > B.Diag.x0 && x < B.Diag.x1 && y > B.Diag.y0 && y < B.Diag.y1)
    {
      if(B.state != ST_OVER) button_set_state(B, ST_OVER);
    } else
    {
      if(B.state == ST_OVER) button_set_state(B, ST_NORMAL);
    }
  }

  for(auto& B: RowsList)
  {
    if (B.state == ST_PRESSED) continue;

    if(x > B.Diag.x0 && x < B.Diag.x1 && y > B.Diag.y0 && y < B.Diag.y1)
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
void gui::event_mouse_btns(int _button, int _action, int _mods)
{
  if(RUN_3D)
  {
    Space3d->mouse_event(_button, _action, _mods);
    return;
  }

  if( (_button == MOUSE_BUTTON_LEFT)
  and (_action == RELEASE) )
  {
    for(auto& B: Buttons)
    {
      if(nullptr == B.caller) continue;
      if(B.state == ST_OVER) B.caller();
    }

    for(auto& B: RowsList)
    {
      if(B.state == ST_OVER)
      {
        // сбросить все в исходное
        for(auto& T: RowsList)
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
void gui::event_keyboard(int key, int scancode, int action, int mods)
{
  if (RUN_3D)
  {
    Space3d->keyboard_event( key, scancode, action, mods);
    if((key == KEY_ESCAPE) && (action == RELEASE)) mode_2d();
  }
  else
  {
    if((key == KEY_ESCAPE) && (action == RELEASE))
      if(!Buttons.empty()) Buttons.back().caller(); // Последняя кнопка ВСЕГДА - "выход/отмена"
  }
}


///
/// \brief gui::character_event
/// \param ch
///
void gui::event_character(uint ch)
{
  // Преобразование целого в строковый символ UTF-8
  std::wstring_convert<std::codecvt_utf8<char32_t>,char32_t> convert;
  std::string Str8 = convert.to_bytes(ch);
  layout L = Cursor->get_layout();
  SymbolsBuffer.emplace_back(std::make_unique<face>(L, Str8, DefaultBgColor));
  L.left += L.width + sym_kerning_default;
  Cursor->update_xy(L);
}


///
/// \brief gui::focus_event
/// \details Потеря окном фокуса в режиме рендера 3D сцены
/// переводит GUI в режим отображения меню
///
void gui::event_focus_lost(void)
{
  if (RUN_3D) mode_2d();
}


///
/// \brief gui::resize_event
/// \param width
/// \param height
///
void gui::event_resize(int w, int h)
{
  LayoutGui.width  = static_cast<uint>(w);
  LayoutGui.height = static_cast<uint>(h);

  // пересчет позции координат прицела (центр окна)
  Cursor3D.x = static_cast<float>(w/2);
  Cursor3D.y = static_cast<float>(h/2);

  RenderBuffer->resize(LayoutGui.width, LayoutGui.height);
  Space3d->resize_event(w, h);

  if(nullptr != current_menu) current_menu();
}


///
/// \brief gui::error_event
/// \param message
///
void gui::event_error(const char* message)
{
  std::cerr << message << std::endl;
}


///
/// \brief gui::window_pos_event
/// \param left
/// \param top
///
void gui::event_reposition(int _left, int _top)
{
  LayoutGui.left = static_cast<uint>(_left);
  LayoutGui.top = static_cast<uint>(_top);
}


///
/// \brief win_data::close_event
///
void gui::event_close(void)
{
  open = false;
}


///
/// \brief gui::textrow
/// \param left
/// \param top
/// \param Text
/// \param symbol_width
/// \param symbol_height
///
void gui::text_append(const layout& L, const std::vector<std::string>& Text, uint kerning = 0)
{
  auto vL = L;
  for(const auto& Symbol: Text)
  {
    SymbolsBuffer.emplace_back(std::make_unique<face>(vL, Symbol, DefaultBgColor));
    vL.left += L.width + kerning;
  }
}


///
/// \brief gui::title
/// \param Label
///
void gui::title(const std::string& Label)
{
  auto b = menu_border_default;
  // Заливка окна фоновым цветом
  //rectangle(b, b, LayoutGui.width - 2 * b, LayoutGui.height - 2 * b, { 0.9f, 1.f, 0.9f, 1.f });
  SymbolsBuffer.emplace_back(std::make_unique<face>(
         layout{ LayoutGui.width - 2 * b, LayoutGui.height - 2 * b, b, b }, " ",
         float_color{ 0.9f, 1.f, 0.9f, 1.f } ));

  auto Text = string2vector(Label);
  uint symbol_width = 14;
  uint symbol_height = 21;
  //rectangle(b, b, LayoutGui.width - 2 * b, title_height_default+1, TitleHemColor);
  SymbolsBuffer.emplace_back(std::make_unique<face>(
         layout{ LayoutGui.width - 2 * b, title_height_default + 1, b, b }, " ",
         TitleHemColor ));

  //rectangle(b, b, LayoutGui.width - 2 * b, title_height_default, TitleBgColor);
  SymbolsBuffer.emplace_back(std::make_unique<face>(
         layout{ LayoutGui.width - 2 * b, title_height_default, b, b }, " ",
         TitleBgColor ));

  uint left = LayoutGui.width/2 - Text.size() * symbol_width / 2;
  uint top =  b + title_height_default/2 - symbol_height/2 + 2;
  text_append({symbol_width, symbol_height, left, top}, Text );
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
  uint left = (LayoutGui.width - btn_width_default)/2;
  uint top = (LayoutGui.height - btn_height_default + title_height_default)/2;

  if(Buttons.empty()) return {left, top};

  // Если размещается 4-я кнопка, то распределяем их в 2 колонки
  if(Buttons.size() == 3)
  {
    uint vert_move_dist = (btn_height_default + btn_padding_default) / 2;
    uint hor_move_dist = (btn_width_default + btn_padding_default) / 2;

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
  uint move_dist = (btn_height_default + btn_padding_default) / 2;
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
void gui::button_move(element& Button, int x, int y)
{
  for(auto& id: Button.Faces )
  {
    SymbolsBuffer[id]->move_xy(x, y);
  }
  auto L = SymbolsBuffer[0]->get_layout();
  Button.Diag.x0 = L.left * 1.0;
  Button.Diag.y0 = L.top * 1.0;
  Button.Diag.x1 = (L.left + L.width) * 1.0;
  Button.Diag.y1 = (L.top + L.height) * 1.0;
}


///
/// \brief gui::button_set_state
/// \param New STATE for the button
///
void gui::button_set_state(element& Button, STATES s)
{
  Button.state = s;

  // Поверхность кнопки, цвет которой надо изменить, состоит из
  // рамки и фоновой заливки. Пока изменим только фоновую заливку
  auto id_face_bg = Button.Faces[1];
  SymbolsBuffer[id_face_bg]->update_rgba(BtnBgColor[s]);
}

///
/// \brief gui::create_element
/// \param Label
/// \param new_caller
/// \return
///
element gui::listrow_make(layout L, const std::string &Label,
                                      const colors& BgColor, const colors& HemColor,
                                      func_ptr new_caller, STATES state = ST_NORMAL)
{
  element Element {};
  Element.caller = new_caller;
  Element.state = state;
  Element.Diag.x0 = L.left * 1.0;
  Element.Diag.y0 = L.top * 1.0;
  Element.Diag.x1 = (L.left + L.width) * 1.0;
  Element.Diag.y1 = (L.top + L.height) * 1.0;

  // рамка элемента
  Element.Faces.push_back(SymbolsBuffer.size());
  SymbolsBuffer.emplace_back(std::make_unique<face>(
         layout{ L.width+2, L.height+2, L.left, L.top }, " ",
         HemColor[Element.state] ));

  // фоновая заливка
  Element.Faces.push_back(SymbolsBuffer.size());
  SymbolsBuffer.emplace_back(std::make_unique<face>(
         layout{ L.width, L.height, L.left+1, L.top+1 }, " ",
         BgColor[Element.state] ));

  // надпись
  auto Text = string2vector(Label);
  uint left = L.left + L.width/2 - Text.size() * (sym_width_default + sym_kerning_default) / 2;
  uint top = L.top + 1 + (L.height - sym_height_default ) / 2;

  for(const auto& Symbol: Text)
  {
    Element.Faces.push_back(SymbolsBuffer.size());
    SymbolsBuffer.emplace_back(std::make_unique<face>(
        layout{ sym_width_default, sym_height_default, left, top },
         Symbol, DefaultBgColor));
    left += sym_width_default + sym_kerning_default;
  }
  return Element;
}


///
/// \brief gui::button_make
/// \param L
/// \param Label
/// \param BgColor
/// \param HemColor
/// \param new_caller
/// \param state
/// \return
///
element gui::button_make(const std::string &Label, ELEMENT_TYPES et, func_ptr new_caller, const STATES state )
{
  auto XY = button_allocation();
  layout L = {btn_width_default, btn_height_default, XY.first, XY.second};

  element Element {};
  Element.element_type = et;
  Element.caller = new_caller;
  Element.state = state;
  Element.Diag.x0 = L.left * 1.0;
  Element.Diag.y0 = L.top * 1.0;
  Element.Diag.x1 = (L.left + L.width) * 1.0;
  Element.Diag.y1 = (L.top + L.height) * 1.0;

  colors BgColors {};
  colors HemColors {};

  switch (et) {
    case GUI_BUTTON:
      BgColors = BtnBgColor;
      HemColors = BtnHemColor;
      break;
    case GUI_LISTROW:
      BgColors = ListBgColor;
      HemColors = ListHemColor;
  }

  // рамка элемента
  Element.Faces.push_back(SymbolsBuffer.size());
  SymbolsBuffer.emplace_back(std::make_unique<face>(
         layout{ L.width+2, L.height+2, L.left, L.top }, " ",
         HemColors[Element.state] ));

  // фоновая заливка
  Element.Faces.push_back(SymbolsBuffer.size());
  SymbolsBuffer.emplace_back(std::make_unique<face>(
         layout{ L.width, L.height, L.left+1, L.top+1 }, " ",
         BgColors[Element.state] ));

  // надпись
  auto Text = string2vector(Label);
  uint left = L.left + L.width/2 - Text.size() * (sym_width_default + sym_kerning_default) / 2;
  uint top = L.top + 1 + (L.height - sym_height_default ) / 2;
  for(const auto& Symbol: Text)
  {
    Element.Faces.push_back(SymbolsBuffer.size());
    SymbolsBuffer.emplace_back(std::make_unique<face>(
        layout{ sym_width_default, sym_height_default, left, top },
        Symbol, DefaultBgColor));
    left += sym_width_default + sym_kerning_default;
  }
  return Element;
}


///
/// \brief gui::list_insert
/// \param String
///
void gui::list_insert(const std::string& String, STATES state = ST_NORMAL)
{
  layout L { };
  L.width = LayoutGui.width - menu_border_default * 4;
  L.height = row_height;
  L.left = menu_border_default * 2;
  L.top = L.left + title_height_default + RowsList.size() * (row_height + 1);
  auto Element = listrow_make(L, String, ListBgColor, ListHemColor, nullptr, state);
  RowsList.push_back(Element);
}


///
/// \brief gui::start_screen
///
void gui::screen_start(void)
{
  clear(); // Очистка всех массивов VAO
  title("Добро пожаловать в TrickRig!");
  Buttons.push_back(button_make("НАСТРОИТЬ", GUI_BUTTON, screen_config));
  Buttons.push_back(button_make("ВЫБРАТЬ КАРТУ", GUI_BUTTON, screen_map_select));
  Buttons.push_back(button_make("ЗАКРЫТЬ", GUI_BUTTON, close));
  current_menu = screen_start;
}


///
/// \brief gui::config_screen
///
void gui::screen_config(void)
{
  clear(); // Очистка всех массивов VAO
  title("ВЫБОР ПАРАМЕТРОВ");
  Buttons.push_back(button_make("ЗАКРЫТЬ", GUI_BUTTON, screen_start ));
  current_menu = screen_config;
}


///
/// \brief gui::select_map
///
void gui::screen_map_select(void)
{
  clear(); // Очистка всех массивов VAO
  title("ВЫБОР КАРТЫ");

  // Составить список карт в каталоге пользователя
  for(auto& it: std::filesystem::directory_iterator(cfg::user_dir()))
    if(std::filesystem::is_directory(it))
    {
      map_current = it.path().string();
      list_insert( cfg::map_name(it.path().string()), ST_PRESSED );
    }

  // DEBUG
  list_insert("debug 1");
  list_insert("debug 2");

  Buttons.push_back(button_make("НОВАЯ КАРТА", GUI_BUTTON, screen_map_new ));
  Buttons.push_back(button_make("УДАЛИТЬ КАРТУ" ));
  Buttons.push_back(button_make("СТАРТ", GUI_BUTTON, map_open ));
  Buttons.push_back(button_make("ОТМЕНА", GUI_BUTTON, screen_start ));

  current_menu = screen_map_select;
}


///
/// \brief gui::callback_timer
/// \details Метод вызывается из цикла рендера каждый кадр
///
void gui::callback_render(void)
{
  if( Cursor != nullptr) update_input();
}


///
/// \brief gui::update_input_cursor
///
void gui::update_input(void)
{
  static const std::chrono::milliseconds pause(300);

  std::chrono::time_point<sys_clock> t_now = sys_clock::now();
  static auto t_last = t_now;

  if (t_now - t_last >= pause)
  {
    t_last = t_now;
    std::string Symbol = " ";
    if(Cursor->uv_equal(Symbol)) Symbol = ":";
    Cursor->update_uv(Symbol);
  }
}


///
/// \brief gui::screen_map_new
///
void gui::screen_map_new(void)
{
  clear(); // Очистка всех массивов VAO
  title("ВВЕДИТЕ НАЗВАНИЕ");

  layout L {};
  L.width = LayoutGui.width - menu_border_default * 4;
  L.height = row_height;
  L.left = menu_border_default * 2;
  L.top = L.left + title_height_default + RowsList.size() * (row_height + 1);

  SymbolsBuffer.emplace_back(std::make_unique<face>(L, " ", ListBgColor[2]));

  L = { sym_width_default, sym_height_default, L.left + 4, L.top + 4 };
  Cursor = std::make_unique<face>(L, ":");

  Buttons.push_back(button_make("СОЗДАТЬ", GUI_BUTTON, current_menu));
  Buttons.push_back(button_make("ОТМЕНА", GUI_BUTTON, current_menu));

  current_menu = screen_map_new;
}


///
/// \brief gui::config_screen
///
void gui::screen_pause(void)
{
  //title("П А У З А");

  clear();
  // Заливка окна фоновым цветом
  auto bx = menu_border_default * 4;
  auto by = 2 * bx;

  SymbolsBuffer.emplace_back(std::make_unique<face>(
         layout{ LayoutGui.width - 2 * bx, LayoutGui.height - 2 * by, bx, by }, " ",
         float_color{ 0.9f, 1.f, 0.9f, 0.5f } ));

  auto Text = string2vector("П А У З А");
  uint symbol_width = 14;
  uint symbol_height = 21;

  SymbolsBuffer.emplace_back(std::make_unique<face>(
         layout{ LayoutGui.width - 2 * bx, title_height_default + 1, bx, by }, " ",
         TitleHemColor ));

  SymbolsBuffer.emplace_back(std::make_unique<face>(
         layout{ LayoutGui.width - 2 * bx, title_height_default, bx, by }, " ",
         TitleBgColor ));

  uint left = LayoutGui.width/2 - Text.size() * symbol_width / 2;
  uint top =  by + title_height_default/2 - symbol_height/2 + 2;
  text_append({symbol_width, symbol_height, left, top}, Text);

  Buttons.push_back(button_make("ПРОДОЛЖИТЬ", GUI_BUTTON, mode_3d));
  Buttons.push_back(button_make("ВЫХОД", GUI_BUTTON, close_map));
  current_menu = screen_pause;
}


///
/// \brief gui::close_map
///
void gui::close_map(void)
{
  cfg::map_view_save(Space3d->ViewFrom, Space3d->look_dir);
  cfg::save(LayoutGui); // Сохранение положения окна
  screen_start();
}


///
/// \brief gui::mode_3d
///
void gui::mode_3d(void)
{
  RUN_3D = true;
  ProgramFrBuf->set_uniform("Cursor", Cursor3D);
  OGLContext->cursor_hide();               // выключить отображение курсора мыши в окне
  OGLContext->set_cursor_pos(LayoutGui.width/2, LayoutGui.height/2);
  hud_enable();
}


///
/// \brief gui::mode_2d
///
void gui::mode_2d(void)
{
  RUN_3D = false;
  ProgramFrBuf->set_uniform("Cursor", {0.f, 0.f, 0.f});
  OGLContext->cursor_restore();
  screen_pause();
}


///
/// \brief gui::create_map
/// \details создается новая карта и сразу открывается
///
void gui::map_create(void)
{
  auto MapDir = cfg::create_map(StringBuffer);
  //Maps.push_back(map(MapDir, StringBuffer));
  //row_selected = Maps.size();     // выбрать номер карты
}


///
/// \brief app::map_open
///
void gui::map_open(void)
{
  Space3d->load(map_current);
  mode_3d();
}


///
/// \brief gui::close_map
///
void gui::map_close(void)
{
  mode_2d();
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

  SymbolsBuffer.emplace_back(std::make_unique<face>(
         layout{ LayoutGui.width, height, 0, LayoutGui.height - height }, " ",
         float_color{ 0.0f, 0.5f, 0.0f, 0.25f } ));

  // FPS
  uint border = 2;
  auto Text = string2vector("FPS:0000, X:+000.0, Y:+000.0, Z:+000.0");
  uint symbol_width = 7;
  uint symbol_height = 7;
  uint row_height = symbol_height + 6;
  uint row_width = Text.size() * (symbol_width + 1);

  SymbolsBuffer.emplace_back(std::make_unique<face>(
         layout{ row_width, row_height, border, border }, " ",
         float_color{0.8f, 0.8f, 1.0f, 0.5f} ));

  uint left = border + row_width/2 - Text.size() * symbol_width / 2;
  uint top =  border + row_height/2 - symbol_height/2 + 1;
  text_append({symbol_width, symbol_height, left, top}, Text);
  fps_uv_data = (gui_indices/6 - 34) * 8 * sizeof(float); // у 34-х символов обновляемая текстура
}


///
/// \brief gui::hud_update
///
void gui::hud_update(void)
{
  // счетчик FPS и координаты камеры
  char line[35] = {'\0'}; // длина строки с '\0'
  std::sprintf(line, "%.4i, X:%+06.1f, Y:%+06.1f, Z:%+06.1f",
               FPS, Space3d->ViewFrom->x, Space3d->ViewFrom->y, Space3d->ViewFrom->z);
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
/// \brief gui::~gui
///
gui::~gui(void)
{
  cfg::save(LayoutGui); // Сохранение положения окна
}

} //namespace tr
