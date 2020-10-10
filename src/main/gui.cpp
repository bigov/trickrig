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
std::string gui::StringBuffer {};   // строка ввода пользователя

static std::unique_ptr<vbo> VBO_xy   = nullptr;    // координаты вершин
static std::unique_ptr<vbo> VBO_rgba = nullptr;    // цвет вершин
static std::unique_ptr<vbo> VBO_uv   = nullptr;    // текстурные координаты

static unsigned int gui_indices = 0; // число индексов в 2Д режиме
func_ptr gui::last_menu = nullptr;

std::unique_ptr<input_ctrl> gui::InputCursor = nullptr; // Текстовый курсор для пользователя

static std::vector<std::unique_ptr<face>> FacesBuf {};  // Массив указателей на 3D элементы меню
static std::vector<element> ActiveElements {};          // Элементы взаимодействия в пользователем

static uint selected_list_row = 0;
static uint top_line = 0;                               // верхняя граница размещения группы элементов GIU

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
face::face(const layout& Layout, const std::string& Symbol, float_color BgColor)
{
#ifndef NDEBUG
  assert(VBO_xy   != nullptr && "VBO_xy не инициализирован" );
  assert(VBO_rgba != nullptr && "VBO_rgba не инициализирован" );
  assert(VBO_uv   != nullptr && "VBO_uv не инициализирован" );
#endif
  init(Layout, Symbol, BgColor);
}


///
/// \brief face::init
/// \param L
/// \param Symbol
/// \param BgColor
///
void face::init(const layout &L, const std::string &Symbol, float_color BgColor)
{
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
/// \brief face::update_rgba
/// \param Color
///
void face::update_rgba(const float_color &Color)
{
  auto Vrgba = rect_rgba(Color);
  VBO_rgba->update(Addr.rgba.size, Vrgba.data(), Addr.rgba.offset);
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
/// \brief char3d::~char3d
///
face::~face(void)
{

  Char.clear();
  Addr.xy.size     = 0;
  Addr.xy.offset   = 0;
  Addr.rgba.size   = 0;
  Addr.rgba.offset = 0;
  Addr.uv.size     = 0;
  Addr.uv.offset   = 0;

  ///
  /// ВАЖНО!!!
  ///
  /// Данные поверхностей располагаются в буфере GPU непрерывным блоком.
  /// Поэтому, при удалении данных одной поверхности фактически происходит
  /// поиск последнего блока данных в конце буфера и его перенос на место
  /// расположения данных удаляемой поверхности.
  ///
  /// Поэтому, если выборочно удалять поверхности из буфера, то необходимо
  /// найти поверхность, данные которой были перенесены и внести изменения в
  /// адрес смещения ее данных в VBO.
  ///
  /*
  GLsizeiptr new_offset = 0;

  if(Addr.xy.size + Addr.xy.offset <= VBO_xy->get_hem())
    new_offset = VBO_xy->remove(Addr.xy.size, Addr.xy.offset);

  fix_face_data_xy(new_offset, Addr.xy.offset);

  if(Addr.rgba.size + Addr.rgba.offset <= VBO_rgba->get_hem())
    new_offset = VBO_rgba->remove(Addr.rgba.size, Addr.rgba.offset);

  fix_face_data_rgba(new_offset, Addr.rgba.offset);

  if(Addr.uv.size + Addr.uv.offset <= VBO_uv->get_hem())
    new_offset = VBO_uv->remove(Addr.uv.size, Addr.uv.offset);

  fix_face_data_uv(new_offset, Addr.uv.offset);

  if(gui_indices >= 6) gui_indices -= 6;
*/
}


///
/// \brief input_ctrl::keyboard_event
/// \param key
/// \param action
///
void input_ctrl::keyboard_event(int key, int /*scancode*/, int action, int /*mods*/)
{
  if((key == KEY_LEFT) && (action == RELEASE)) move_left();
  else if((key == KEY_RIGHT) && (action == RELEASE)) move_right();
}


///
/// \brief input_ctrl::move_next
/// \return
///
bool input_ctrl::move_next(uint symbol_size)
{
  if(row_size >= row_limit) return false;

  row_size += 1;
  row_position += 1;

  SizeOfSymbols.push_back(symbol_size);

  Layout.left += Layout.width + sym_kerning_default;
  update_xy(Layout);

  return true;
}


///
/// \brief input_ctrl::current_pos
/// \return
///
uint input_ctrl::current_char(void)
{
  uint result = 0;
  for(uint i = 0; i < row_position; i++) result += SizeOfSymbols[i];
  return result;
}


///
/// \brief input_ctrl::move_left
///
void input_ctrl::move_left(void)
{
  if(row_position == 0) return;

  Layout.left -= (Layout.width + sym_kerning_default);
  update_xy(Layout);

  row_position -= 1;
}


///
/// \brief input_ctrl::move_right
///
void input_ctrl::move_right(void)
{
  if(row_position >= row_size) return;

  row_position += 1;
  Layout.left += Layout.width + sym_kerning_default;
  update_xy(Layout);
}


///
/// \brief input_ctrl::blink
///
void input_ctrl::blink(void)
{
  if(visible) update_rgba(DefaultBgColor);
  else update_rgba(ColorBgOn);
  visible = !visible;
}


///
/// \brief elements_group::elements_group
/// \param _type
///
group::group(ELEMENT_TYPES _type): element_type(_type)
{
  Params.clear();
};


///
/// \brief elements_group::append
/// \param Label
/// \param state
/// \param callback
///
void group::append(const std::string& newLabel, func_ptr callback, STATES state, bool dependant)
{
  Params.emplace_back( params { newLabel, callback, state, dependant, 0, 0 } );
}


///
/// \brief group::buttons_align
///
void group::buttons_align(void)
{
  auto items_count = Params.size();

#ifndef NDEBUG
  assert(items_count < 7 && "Больше 6 кнопок располагать запрещено");
#endif

  auto el_w = btn_width_default + btn_padding_default;  // ширина элемента
  auto el_h = btn_height_default + btn_padding_default; // высота элемента
  uint gr_w = el_w * 3;                                 // ширина группы
  uint gr_h = el_h;                                     // высота группы

  uint i_max = 3;
  if(items_count < i_max)
  {
    gr_w = el_w * items_count;
    i_max = items_count;
  }

  if(items_count > 3) gr_h += el_h;

  uint left = (LayoutGui.width - gr_w) / 2;
  top_line = (LayoutGui.height + top_line - gr_h) / 2;

  uint i = 0;
  for(; i < i_max; ++i)
  {
    Params[i].left = i * el_w + left;
    Params[i].top = top_line;
  }

  top_line += el_h;
  if(items_count < 4) return;

  // Если кнопок от 4 до 6, то располагаем их в два ряда.
  //Cмещаем нижний ряд кнопок, чтобы он был по центру
  uint shift = (6 - items_count) * el_w / 2;

  for(; i < items_count; ++i)
  {
    Params[i].left = Params[i - 3].left + shift;
    Params[i].top = top_line;
  }

  top_line += el_h;
  return;
}


///
/// \brief group::listrows_align
///
void group::listrows_align(void)
{
  auto items_count = Params.size();

#ifndef NDEBUG
  assert(items_count < 10 && "Больше 9 строк использовать запрещено");
#endif

  uint left = menu_border_default * 1.2f;
  top_line += listrow_height_default;

  for(uint i = 0; i < items_count; ++i)
  {
    Params[i].left = left;
    Params[i].top = top_line;
    top_line += listrow_height_default;
  }
}


///
/// \brief group::make_button
/// \param P
/// \return
///
void group::make_buttons(void)
{
  buttons_align();

  for(const auto& P: Params)
  {
    layout L {};
    element Element {};
    colors BgColors {};
    colors HemColors {};
    BgColors = BtnBgColor;
    HemColors = BtnHemColor;
    L.width = btn_width_default;
    L.height = btn_height_default;
    L.left = P.left;
    L.top = P.top;

    Element.element_type = element_type;
    Element.caller = P.caller;
    Element.state = P.state;
    Element.Margins.x0 = L.left * 1.0;
    Element.Margins.y0 = L.top * 1.0;
    Element.Margins.x1 = (L.left + L.width) * 1.0;
    Element.Margins.y1 = (L.top + L.height) * 1.0;

    // рамка элемента
    Element.FacesID.push_back(FacesBuf.size());
    FacesBuf.emplace_back(std::make_unique<face>(
                            layout{ L.width, L.height, L.left, L.top }, " ",
                            HemColors[Element.state] ));

    // фоновая заливка
    Element.FacesID.push_back(FacesBuf.size());
    FacesBuf.emplace_back(std::make_unique<face>(
         layout{ L.width-2, L.height-2, L.left+1, L.top+1 }, " ",
         BgColors[Element.state] ));

    // надпись
    auto Text = string2vector(P.Label);
    uint left = L.left + L.width/2 - Text.size() * (sym_width_default + sym_kerning_default) / 2;
    uint top = L.top + 1 + (L.height - sym_height_default ) / 2;

    for(const auto& Symbol: Text)
    {
      Element.FacesID.push_back(FacesBuf.size());
      FacesBuf.emplace_back(std::make_unique<face>(
        layout{ sym_width_default, sym_height_default, left, top },
        Symbol, DefaultBgColor));
      left += sym_width_default + sym_kerning_default;
    }

    ActiveElements.push_back(Element);
  }
}


///
/// \brief group::make_rowlist
/// \param P
/// \return
///
void group::make_listrow(void)
{
  listrows_align();
  uint id = 0;

  for(const auto& P: Params)
  {
    layout L {};
    element Element {};
    colors BgColors {};
    colors HemColors {};

    BgColors = ListBgColor;
    HemColors = ListHemColor;
    L.width = LayoutGui.width - menu_border_default * 4;
    L.height =  listrow_height_default;
    L.left = menu_border_default * 2;
    L.top = L.left + title_height_default + ActiveElements.size() * (listrow_height_default + 1);

    Element.element_type = element_type;
    Element.caller = P.caller;
    Element.id = id++;
    Element.state = P.state;
    Element.Margins.x0 = L.left * 1.0;
    Element.Margins.y0 = L.top * 1.0;
    Element.Margins.x1 = (L.left + L.width) * 1.0;
    Element.Margins.y1 = (L.top + L.height) * 1.0;

    // рамка элемента
    Element.FacesID.push_back(FacesBuf.size());
    FacesBuf.emplace_back(std::make_unique<face>(
         layout{ L.width, L.height, L.left, L.top }, " ",
         HemColors[Element.state] ));

    // фоновая заливка
    Element.FacesID.push_back(FacesBuf.size());
    FacesBuf.emplace_back(std::make_unique<face>(
         layout{ L.width-2, L.height-2, L.left+1, L.top+1 }, " ",
         BgColors[Element.state] ));

    // надпись
    auto Text = string2vector(P.Label);
    uint left = L.left + L.width/2 - Text.size() * (sym_width_default + sym_kerning_default) / 2;
    uint top = L.top + 1 + (L.height - sym_height_default ) / 2;

    for(const auto& Symbol: Text)
    {
      Element.FacesID.push_back(FacesBuf.size());
      FacesBuf.emplace_back(std::make_unique<face>(
                              layout{ sym_width_default, sym_height_default, left, top },
                              Symbol, DefaultBgColor));
      left += sym_width_default + sym_kerning_default;
    }
    ActiveElements.push_back(Element);
  }
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
  screen_start(0);

  while(render()) continue;
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

// настройка положения атрибутов GLSL программы
for(auto& A: ProgramFrBuf->AtribsList)
  VboWin.set_attrib(A.index, A.d_size, A.type, A.normalized, A.stride, A.pointer);


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
  VBO_xy->set_attrib(A->index, A->d_size, A->type, A->normalized, 0, 0);
  ++A;
  VBO_rgba->set_attrib(A->index, A->d_size, A->type, A->normalized, 0, 0);
  ++A;
  VBO_uv->set_attrib(A->index, A->d_size, A->type, A->normalized, 0, 0);

  glBindVertexArray(0);
  Program2d->unuse();
}


///
/// \brief gui::vbo_clear
///
void gui::clear(void)
{
  FacesBuf.clear();
  ActiveElements.clear();
  top_line = 0;
  VBO_xy->clear();
  VBO_uv->clear();
  VBO_rgba->clear();
  gui_indices = 0;
  InputCursor = nullptr;
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

  for(auto& B: ActiveElements)
  {
    if (B.state == ST_PRESSED) continue;
    if (B.state == ST_DISABLE) continue;

    if(x > B.Margins.x0 && x < B.Margins.x1 && y > B.Margins.y0 && y < B.Margins.y1)
    {
      if(B.state == ST_NORMAL) element_set_state(B, ST_OVER);
    } else
    {
      if(B.state == ST_OVER) element_set_state(B, ST_NORMAL);
    }
  }
}


///
/// \brief gui::select_row
/// \param id
///
void gui::select_row(uint id)
{
  selected_list_row = id;

  for(auto& B: ActiveElements)
  {
    if(B.state == ST_DISABLE) element_set_state(B, ST_NORMAL);
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

  // Переключение элемента, над которым нажата левая кнопка мыши
  if((_button == MOUSE_BUTTON_LEFT) and (_action == PRESS))
  {
    for(auto& B: ActiveElements)
    {
      if(B.state == ST_OVER) element_set_state(B, ST_PRESSED);
      else if(B.state == ST_PRESSED) element_set_state(B, ST_NORMAL);
    }
  }

  // При отпускании вызываем обработчик
  if((_button == MOUSE_BUTTON_LEFT) and (_action == RELEASE))
  {
    for(auto& B: ActiveElements)
    {
      if(B.state == ST_PRESSED) if(nullptr != B.caller) B.caller(B.id);
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
    if((key == KEY_ESCAPE) && (action == RELEASE) && (!ActiveElements.empty()))
       ActiveElements.back().caller(0); // Последняя кнопка ВСЕГДА - "выход/отмена"

    InputCursor->keyboard_event(key, scancode,action, mods);
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
  std::string Str8 = convert.to_bytes(ch);              // Введенный символ (8 или 16 бит)

  // Текущие данные курсора
  layout CursorLayout = InputCursor->get_layout();      // координаты курсора
  uint char_position = InputCursor->current_char();     // позиция символа для вставки текста
  uint row_position = InputCursor->get_row_position();  // текущую позицию ввода
  uint row_size = InputCursor->get_row_size();          // размер строки

  // Переместить курсор на следующую позицию в строке
  if (!InputCursor->move_next(Str8.size())) return;

  // Вставить введенный символ в текстовую строку в позиции курсора
  StringBuffer.insert(char_position, Str8);

  // 3D элементы, отображающие вводимые пользователем символы, расположены в
  // конце буфера.Значение указателя вектора на символ под курсором ввода
  // соответствует разности между длинной строки и номером позиции курсора:
  auto it = FacesBuf.end() - (row_size - row_position);
  // Вставить введенный символ в 3D буфер по месту положения указателя
  FacesBuf.emplace(it, std::make_unique<face>(CursorLayout, Str8, DefaultBgColor));
  // Переключить указатель на следующий элемент
  ++it;
  // Все элементы от указателя до конца буфера сдвинуть на одну позицию вправо
  for(; it < FacesBuf.end(); it++)
    (*it)->move_xy(sym_width_default + sym_kerning_default, 0);
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

  if(nullptr != last_menu) last_menu(0);
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
    FacesBuf.emplace_back(std::make_unique<face>(vL, Symbol, DefaultBgColor));
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
  FacesBuf.emplace_back(std::make_unique<face>(
         layout{ LayoutGui.width - 2 * b, LayoutGui.height - 2 * b, b, b }, " ",
         float_color{ 0.9f, 1.f, 0.9f, 1.f } ));

  // Создание рамки
  FacesBuf.emplace_back(std::make_unique<face>(
         layout{ LayoutGui.width - 2 * b, title_height_default + 1, b, b }, " ",
         TitleHemColor ));

  // Фоновая заливка заголовка
  FacesBuf.emplace_back(std::make_unique<face>(
         layout{ LayoutGui.width - 2 * b, title_height_default, b, b }, " ",
         TitleBgColor ));

  auto Text = string2vector(Label);
  uint symbol_width = 14;
  uint symbol_height = 21;
  uint left = LayoutGui.width/2 - Text.size() * symbol_width / 2;
  uint top =  b + title_height_default/2 - symbol_height/2 + 2;
  text_append({symbol_width, symbol_height, left, top}, Text );

  top_line = title_height_default + menu_border_default;
}


///
/// \brief gui::button_move
/// \param Button
/// \param x
/// \param y
///
void gui::element_move(element& Elm, int x, int y)
{
  if(Elm.FacesID.empty()) return;

  // Передвинуть все поверхности
  for(auto& id: Elm.FacesID ) FacesBuf[id]->move_xy(x, y);

  // Координаты рамки
  auto L = FacesBuf[Elm.FacesID.front()]->get_layout();

  // Обновить значения границ элемента в окне
  Elm.Margins.x0 = L.left * 1.0;
  Elm.Margins.y0 = L.top * 1.0;
  Elm.Margins.x1 = (L.left + L.width) * 1.0;
  Elm.Margins.y1 = (L.top + L.height) * 1.0;
}


///
/// \brief gui::button_set_state
/// \param New STATE for the button
///
void gui::element_set_state(element& El, STATES s)
{
  El.state = s;

  // Цвет элемента которой надо изменить, состоит из
  // рамки и фоновой заливки. Пока изменим только фоновую заливку
  auto id_face_bg = El.FacesID[1];

  switch (El.element_type){
    case GUI_BUTTON:
      FacesBuf[id_face_bg]->update_rgba(BtnBgColor[s]);
      break;
    case GUI_ROWSLIST:
      FacesBuf[id_face_bg]->update_rgba(ListBgColor[s]);
      break;
  }
}


///
/// \brief gui::callback_render
/// \details Метод вызывается из цикла рендера каждый кадр
///
void gui::callback_render(void)
{
  if(InputCursor != nullptr) update_input();
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
    InputCursor->blink();
  }
}


///
/// \brief gui::start_screen
///
void gui::screen_start(uint)
{
  clear();
  title("Добро пожаловать в TrickRig!");

  group Buttons { GUI_BUTTON };
  Buttons.append("НАСТРОИТЬ", screen_config);
  Buttons.append("ВЫБРАТЬ КАРТУ", screen_map_select);
  Buttons.append("ЗАКРЫТЬ", close);
  Buttons.make_buttons();

  last_menu = screen_start;
}


///
/// \brief gui::config_screen
///
void gui::screen_config(uint)
{
  clear(); // Очистка всех массивов VAO
  title("ВЫБОР ПАРАМЕТРОВ");

  group Buttons { GUI_BUTTON };
  Buttons.append("ЗАКРЫТЬ", screen_start );
  Buttons.make_buttons();
  last_menu = screen_config;
}


///
/// \brief gui::select_map
///
void gui::screen_map_select(uint)
{
  clear(); // Очистка всех массивов VAO
  title("ВЫБОР КАРТЫ");

  group ListMaps {GUI_ROWSLIST};
  // Составить список названий карт из каталога пользователя
  auto DirList = std::filesystem::directory_iterator(cfg::app_data_dir());


  for(auto& it: DirList)
    if(std::filesystem::is_directory(it))
    {
      ListMaps.append(cfg::map_name(it.path().string()), select_row, ST_NORMAL);
    }

  ListMaps.make_listrow();

  group Buttons { GUI_BUTTON };
  Buttons.append("НОВАЯ КАРТА", screen_map_new);
  Buttons.append("УДАЛИТЬ КАРТУ", nullptr, ST_DISABLE);
  Buttons.append("СТАРТ", map_open, ST_DISABLE);
  Buttons.append("ОТМЕНА", screen_start);
  Buttons.make_buttons();

  last_menu = screen_map_select;
}


///
/// \brief gui::screen_map_new
///
void gui::screen_map_new(uint)
{
  clear(); // Очистка всех массивов VAO
  title("ВВЕДИТЕ НАЗВАНИЕ");

  layout L {};
  L.width = LayoutGui.width - menu_border_default * 4;
  L.height = listrow_height_default;
  L.left = menu_border_default * 2;
  L.top = L.left + title_height_default + ActiveElements.size() * (listrow_height_default + 1);

  FacesBuf.emplace_back(std::make_unique<face>(L, " ", ListBgColor[2]));

  L = { sym_width_default, sym_height_default, L.left + 4, L.top + 4 };
  InputCursor = std::make_unique<input_ctrl>(L);

  group Buttons {GUI_BUTTON };
  Buttons.append("СОЗДАТЬ", map_create);
  Buttons.append("ОТМЕНА", last_menu);
  Buttons.make_buttons();
  last_menu = screen_map_new;
  StringBuffer.clear();
}


///
/// \brief gui::config_screen
///
void gui::screen_pause(uint)
{
  //title("П А У З А");

  clear();
  // Заливка окна фоновым цветом
  auto bx = menu_border_default * 4;
  auto by = 2 * bx;

  FacesBuf.emplace_back(std::make_unique<face>(
         layout{ LayoutGui.width - 2 * bx, LayoutGui.height - 2 * by, bx, by }, " ",
         float_color{ 0.9f, 1.f, 0.9f, 0.5f } ));

  auto Text = string2vector("П А У З А");
  uint symbol_width = 14;
  uint symbol_height = 21;

  FacesBuf.emplace_back(std::make_unique<face>(
         layout{ LayoutGui.width - 2 * bx, title_height_default + 1, bx, by }, " ",
         TitleHemColor ));

  FacesBuf.emplace_back(std::make_unique<face>(
         layout{ LayoutGui.width - 2 * bx, title_height_default, bx, by }, " ",
         TitleBgColor ));

  uint left = LayoutGui.width/2 - Text.size() * symbol_width / 2;
  uint top =  by + title_height_default/2 - symbol_height/2 + 2;
  text_append({symbol_width, symbol_height, left, top}, Text);
  top += title_height_default;

  group Buttons { GUI_BUTTON };
  Buttons.append("ПРОДОЛЖИТЬ", mode_3d);
  Buttons.append("ВЫХОД", close_map);
  Buttons.make_buttons();
  last_menu = screen_pause;
}


///
/// \brief gui::close_map
///
void gui::close_map(uint)
{
  cfg::map_view_save(Space3d->ViewFrom, Space3d->look_dir);
  cfg::save(LayoutGui); // Сохранение положения окна
  screen_start(0);
}


///
/// \brief gui::mode_3d
///
void gui::mode_3d(uint)
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
  screen_pause(0);
}


///
/// \brief gui::create_map
///
void gui::map_create(uint)
{
  auto MapDir = cfg::create_map(StringBuffer);
  screen_map_select(0);
}


///
/// \brief app::map_open
///
void gui::map_open(uint)
{
  std::vector<std::string> Maps {};
  // Список карт в каталоге пользователя
  auto DirList = std::filesystem::directory_iterator(cfg::app_data_dir());

  for(auto& it: DirList)
    if(std::filesystem::is_directory(it))
      Maps.push_back(it.path().string());

  if(Maps.size() < selected_list_row) ERR("\nОшибка выбора карты");

  Space3d->load(Maps[selected_list_row]);
  mode_3d(0);
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

  FacesBuf.emplace_back(std::make_unique<face>(
         layout{ LayoutGui.width, height, 0, LayoutGui.height - height }, " ",
         float_color{ 0.0f, 0.5f, 0.0f, 0.25f } ));

  // FPS
  uint border = 2;
  auto Text = string2vector("FPS:0000, X:+000.0, Y:+000.0, Z:+000.0");
  uint symbol_width = 7;
  uint symbol_height = 7;
  uint row_height = symbol_height + 6;
  uint row_width = Text.size() * (symbol_width + 1);

  FacesBuf.emplace_back(std::make_unique<face>(
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
  clear();
  glUseProgram(0);
  glBindTexture(GL_TEXTURE_2D, 0);
  ProgramFrBuf = nullptr;
  Program2d = nullptr;
  Space3d = nullptr;
  RenderBuffer = nullptr;
  OGLContext = nullptr;
}

} //namespace tr
