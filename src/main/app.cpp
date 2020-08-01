#include "app.hpp"

namespace tr {

#define MAX_VERTEX_BUFFER 512 * 1024
#define MAX_ELEMENT_BUFFER 128 * 1024

GLuint      app::texture_gui = 0;      // id тектуры HUD
layout      app::Layout {};            // размеры и положение окна
std::vector<map> app::Maps {};         // список карт
uchar_color app::color_title {0xFF, 0xFF, 0xDD, 0xFF}; // фон заголовка
size_t      app::row_selected = 0;     // какая строка выбрана
double      app::mouse_x = 0.0;        // позиция указателя относительно левой границы
double      app::mouse_y = 0.0;        // позиция указателя относительно верхней границы
int         app::mouse_left = EMPTY;   // нажатие на левую кнопку мыши

std::unique_ptr<space_3d> app::Space3d = nullptr;
std::unique_ptr<gui> app::AppGUI = nullptr;

app::MENU_MODES app::MenuMode = SCREEN_START;    // режим окна приложения
bool app::RUN_3D = false;
glm::vec3 app::Cursor3D = { 200.f, 200.f, 2.f }; // положение и размер прицела
std::unique_ptr<glsl> app::ShowScene = nullptr;  // Вывод текстуры фреймбуфера на окно

GLuint app::vao2d  = 0;

std::unique_ptr<glsl> Program2d = nullptr;            // построение 2D элементов
std::unique_ptr<frame_buffer> RenderBuffer = nullptr; // рендер-буфер окна

///
/// \brief init_prog_2d
///
void init_prog_2d(void)
{
  std::list<std::pair<GLenum, std::string>> Shaders {};
  Shaders.push_back({ GL_VERTEX_SHADER, "assets\\shaders\\2d_vert.glsl" });
  Shaders.push_back({ GL_FRAGMENT_SHADER, "assets\\shaders\\2d_frag.glsl" });

  Program2d = std::make_unique<glsl>(Shaders);
  Program2d->use();

  // VBO2d_base Обработка массива с данными 2D-координат и цвета вершин
  GLsizei stride = sizeof(GLfloat) * 6;
  Program2d->AtribsList.push_back({Program2d->attrib("vCoordXY"), 2, GL_FLOAT, GL_TRUE, stride, 0 * sizeof(GLfloat)});
  Program2d->AtribsList.push_back({Program2d->attrib("vColor"), 4, GL_FLOAT, GL_TRUE, stride, 2 * sizeof(GLfloat)});

  // VBO2d_uv Массив текстурных координат UV заполняется отдельно. Может меняться динамически.
  Program2d->AtribsList.push_back({Program2d->attrib("vCoordUV"), 2, GL_FLOAT, GL_TRUE, 0, 0});

  glUniform1i(Program2d->uniform("font_texture"), 4);  // glActiveTexture(GL_TEXTURE4)

  Program2d->unuse();
}


///
/// \brief load_font_texture
/// \details  Загрузка текстуры шрифта
///
void load_font_texture(void)
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
}


///
///
///
app::app(void)
{
  std::string title = std::string(APP_NAME) + " v." + std::string(APP_VERSION);

#ifndef NDEBUG
  if(strcmp(USE_CLANG, "FALSE") == 0) title += " [GCC]";
  else title += " [Clang]";
  title += " (debug mode)";
#endif

  std::clog << "Start: " << title << std::endl;

  GLContext = std::make_shared<trgl>(title.c_str());
  layout_set(cfg::WinLayout);
  GLContext->set_window(Layout.width, Layout.height, MIN_GUI_WIDTH, MIN_GUI_HEIGHT, Layout.left, Layout.top);

  init_prog_2d();      // Шейдерная программа для построения 2D элементов пользовательского интерфейса
  load_font_texture(); // Загрузчик текстуры со шрифтом

  // настройка рендер-буфера
  RenderBuffer = std::make_unique<frame_buffer>(Layout.width, Layout.height);

  Space3d = std::make_unique<space_3d>(GLContext);
  AppGUI = std::make_unique<gui>(GLContext, Space3d->ViewFrom);

  TimeStart = std::chrono::system_clock::now();

  // Составить список карт в каталоге пользователя
  auto MapsDirs = dirs_list(cfg::user_dir()); // список директорий с картами
  for(auto &P: MapsDirs) { Maps.push_back(map(P, cfg::map_name(P))); }

  glActiveTexture(GL_TEXTURE2);     // текстура для GUI (меню приложения)
  glGenTextures(1, &texture_gui);
  glBindTexture(GL_TEXTURE_2D, texture_gui);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

  /// Инициализация GLSL программы обработки текстуры фреймбуфера.
  ///
  /// Текстура фрейм-буфера за счет измения порядка следования координат
  /// вершин с 1-2-3-4 на 3-4-1-2 перевернута - верх и низ в сцене
  /// меняются местами. Благодаря этому, нулевой координатой (0,0) окна
  /// становится более привычный верхний-левый угол, и загруженные из файла
  /// изображения текстур применяются без дополнительного переворота.

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

  ShowScene = std::make_unique<glsl>(Shaders);
  ShowScene->use();
  ShowScene->AtribsList.push_back(
    { ShowScene->attrib("position"), 2, GL_FLOAT, GL_TRUE, vertex_bytes, 0 * sizeof(GLfloat) });
  ShowScene->AtribsList.push_back(
    { ShowScene->attrib("texcoord"), 2, GL_FLOAT, GL_TRUE, vertex_bytes, 2 * sizeof(GLfloat) });
  glUniform1i(ShowScene->uniform("WinTexture"), 1); // GL_TEXTURE1 - фрейм-буфер
  ShowScene->set_uniform("Cursor", {0.f, 0.f, 0.f});
  ShowScene->unuse();

  vbo VboWin { GL_ARRAY_BUFFER };
  VboWin.allocate( sizeof(WinData), WinData );
  VboWin.set_attributes(ShowScene->AtribsList); // настройка положения атрибутов GLSL программы

  glBindVertexArray(0);
}


///
/// \brief gui::~gui
///
app::~app(void)
{
  cfg::save(Layout); // Сохранение положения окна
}


void app::mode_3d(void)
{
  RUN_3D = true;
  ShowScene->set_uniform("Cursor", Cursor3D);
}


void app::mode_2d(void)
{
  RUN_3D = false;
  ShowScene->set_uniform("Cursor", {0.f, 0.f, 0.f});
}


///
/// \brief gui::add_text_cursor
/// \param _Fn       шрифт ввода
/// \param _Dst      строка ввода
/// \param position  номер позиции курсора в строке ввода
/// \details Формирование курсора ввода, моргающего с интервалом в пол-секунды
///
void app::cursor_text_row(const atlas &_Fn, image &_Dst, size_t position)
{
  uchar_color c = {0x11, 0xDD, 0x00, 0xFF};
  auto tm = std::chrono::duration_cast<std::chrono::milliseconds>
      ( std::chrono::system_clock::now()-TimeStart ).count();

  auto tc = trunc(tm/1000) * 1000;
  if(tm - tc > 500) c.a = 0xFF;
  else c.a = 0x00;

  image Cursor {3, _Fn.get_cell_height(), c};
  Cursor.put(_Dst, _Fn.get_cell_width() * (position + 1) + 1,
              (_Dst.get_height() - _Fn.get_cell_height()) / 2 );
}


///
/// \brief gui::draw_text_row
/// \param id
/// \param x - координата относительно окна приложения
/// \param y - координата относительно окна приложения
/// \param w
/// \param h
/// \param text
/// \details Отобажение тестовой строки, реагирующей на указатель мыши
///
//void app::row_text(size_t id, uint x, uint y, uint w, uint h, const std::string &text)
void app::row_text(size_t, uint, uint, uint, uint, const std::string&)
{

  /*uchar_color
    normal = color_title,                   // обычный цвет
    over = { 0xFF, 0xFF, 0xFF, 0xFF },      // цвет строки когда курсор над строкой
    selected = { 0xDD, 0xFF, 0xDD, 0xFF };  // цвет выбранной строки

  if(row_selected == id) // если строка уже выбрана, то ее цвет всегда "selected"
  {                       // не зависимо от положения на экране указателя мыши
    normal = selected;
    over = selected;
  }
  uchar_color bg_color = normal;

  // Если указатель находится над строкой
  if(mouse_x >= x && mouse_x <= x+w && mouse_y >= y && mouse_y <= y+h)
  {
    element_over = ROW_MAP_NAME; // для обработки в "menu_selector"
    bg_color = over;

    if(mouse_left == PRESS)
    {
      row_selected = id;
      x += 1; y += 1;
    }
    else {
      x -= 1; y -= 1;
    }
  }

  image Row { w, h, bg_color };
  textstring_place(Font18n, text, Row, Font18n.get_cell_width()/2, 6);
  Row.put(MainMenu, x, y);
  */
}


///
/// \brief gui::cancel Отмена текущего режима
///
void app::cancel(void)
{
  key    = EMPTY;
  action = EMPTY;

  if(RUN_3D)
  {
    cfg::map_view_save(Space3d->ViewFrom, Space3d->look_dir);
    mode_2d();
    GLContext->cursor_restore();             // Включить указатель мыши
    GLContext->set_cursor_observer(*this);   // переключить обработчик смещения курсора
    GLContext->set_mbutton_observer(*this);   // обработчик кнопок мыши
    GLContext->set_keyboard_observer(*this); // и клавиатуры
    return;
  }

  switch (MenuMode)
  {
    case SCREEN_LSELECT:
      MenuMode = SCREEN_START;
      break;
    case SCREEN_CREATE:
      MenuMode = SCREEN_LSELECT;
      break;
    case SCREEN_CONFIG:
      MenuMode = SCREEN_START;
      break;
    case SCREEN_START:
      AppGUI->open = false;
      break;
  }
}


///
/// \brief gui::create_map
/// \details создается новая карта и сразу открывается
///
void app::create_map(void)
{
  auto MapDir = cfg::create_map(StringBuffer);
  Maps.push_back(map(MapDir, StringBuffer));
  row_selected = Maps.size();     // выбрать номер карты
}


///
/// \brief gui::remove_map
///
void app::remove_map(void)
{
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
}


///
/// \brief app::app_close
///
void app::app_close(void)
{
  AppGUI->open = false;
}


///
/// \brief app::map_open
///
void app::map_open(uint map_id)
{
  assert((map_id < Maps.size()) && "Map id out of range");

  cfg::map_view_load(Maps[map_id].Folder, Space3d->ViewFrom, Space3d->look_dir);
  Space3d->load();
  AppGUI->hud_enable();
  mode_3d();
}


///
/// \brief Создание элементов интерфейса окна
///
/// \details Окно приложения может иметь два состояния: HUD-3D, в котором
/// происходит основной процесс, и режим настройка/управление (menu). В
/// обоих режимах поверх основного изображения OpenGL сцены накладываются
/// изображения дополнительных элементов. В первом случае это HUD и
/// контрольные элементы взаимодействия с контентом сцены, во втором -
/// это GIU элементы меню.
///
/// Из всех необходимых элементов собирается общее графическое изображение в
/// виде текстурного массива и передается для рендера в OpenGL
///
void app::show(void)
{

  GLContext->set_char_observer(*this);      // ввод с клавиатуры
  GLContext->set_error_observer(*this);     // отслеживание ошибок

  //GLContext->set_cursor_observer(*this);    // курсор мыши в окне
  GLContext->set_cursor_observer(*AppGUI.get());    // курсор мыши в окне

  //GLContext->set_mbutton_observer(*this);   // кнопки мыши
  GLContext->set_mbutton_observer(*AppGUI.get());   // кнопки мыши

  GLContext->set_keyboard_observer(*this);  // клавиши клавиатуры
  GLContext->set_position_observer(*this);  // положение окна
  GLContext->add_size_observer(*this);      // размер окна
  GLContext->set_close_observer(*this);     // закрытие окна
  GLContext->set_focuslost_observer(*this); // потеря окном фокуса ввода

  AppGUI->open = true;

  while(AppGUI->open)
  {
    if(RUN_3D) Space3d->render(); // рендер 3D сцены
    AppGUI->render();
    window_frame_render();
  }
}


///
/// \brief app::window_frame_render
/// \details
/// Кадр сцены рендерится в изображение на (2D) "холсте" фреймбуфера,
/// или текстуре интерфейса меню. После этого изображение в виде
/// текстуры накладывается на прямоугольник окна приложения.
///
void app::window_frame_render(void)
{
  ShowScene->use();
  vbo_mtx.lock();
  glBindVertexArray(vao2d);
  glDrawArrays(GL_TRIANGLES, 0, 6);
  GLContext->swap_buffers();
  vbo_mtx.unlock();
  ShowScene->unuse();

#ifndef NDEBUG
  CHECK_OPENGL_ERRORS
#endif
}

///
/// \brief gui::window_pos_event
/// \param left
/// \param top
///
void app::reposition_event(int _left, int _top)
{
  Layout.left = static_cast<uint>(_left);
  Layout.top = static_cast<uint>(_top);
}


///
/// \brief gui::resize_event
/// \param width
/// \param height
///
void app::resize_event(int w, int h)
{
  assert(w >= 0);
  assert(h >= 0);

  Layout.width  = static_cast<uint>(w);
  Layout.height = static_cast<uint>(h);

  // пересчет позции координат прицела (центр окна)
  Cursor3D.x = static_cast<float>(w/2);
  Cursor3D.y = static_cast<float>(h/2);

  // пересчет размеров изображения GUI
}


///
/// \brief gui::character_event
/// \param ch
///
void app::character_event(uint ch)
{
  if(!text_mode) return;

  if(ch < 128)
  {
    StringBuffer += char(ch);
  }
  else
  {
    auto str = wstring2string({static_cast<wchar_t>(ch)});
    if(str == "№") str = "N";     // № трехбайтный, поэтому заменим на N
    if(str.size() > 2) str = "_"; // блокировка 3-х байтных символов
    StringBuffer += str;
  }
}


///
/// \brief win_data::close_event
///
void app::close_event(void)
{
  AppGUI->open = false;
}


///
/// \brief gui::error_event
/// \param message
///
void app::error_event(const char* message)
{
  std::cerr << message << std::endl;
}


///
/// \brief gui::mouse_event
/// \param _button
/// \param _action
/// \param _mods
///
void app::mouse_event(int _button, int _action, int _mods)
{
  //func_ptr caller = MenuOnImage.mouse_event(_button, _action, _mods);
  //update_gui_image();
  //if ( caller != nullptr ) caller();
  AppGUI->mouse_event(_button, _action, _mods);
  map_open(0);
}


///
/// \brief gui::keyboard_event
/// \param key
/// \param scancode
/// \param action
/// \param mods
///
void app::keyboard_event(int _key, int _scancode, int _action, int _mods)
{
  key      = _key;
  scancode = _scancode;
  action   = _action;
  mods     = _mods;
  if((key == KEY_ESCAPE) && (action == RELEASE)) cancel();
  if (RUN_3D) Space3d->keyboard_event(_key, _scancode, _action, _mods);
  else AppGUI->keyboard_event(_key, _scancode, _action, _mods);
}


///
/// \brief gui::focus_event
/// \details Потеря окном фокуса в режиме рендера 3D сцены
/// переводит GUI в режим отображения меню
///
void app::focus_lost_event()
{
  if (RUN_3D)
  {
     cfg::map_view_save(Space3d->ViewFrom, Space3d->look_dir);
     mode_2d();
     GLContext->cursor_restore();            // Включить указатель мыши
     GLContext->set_cursor_observer(*this);  // переключить обработчик смещения курсора
     GLContext->set_mbutton_observer(*this);  // обработчик кнопок мыши
  }
}


///
/// \brief gui::set_location
/// \param width
/// \param height
/// \param left
/// \param top
///
void app::layout_set(const layout &L)
{
  Layout = L;
  Cursor3D.x = static_cast<float>(L.width/2);
  Cursor3D.y = static_cast<float>(L.height/2);
  aspect  = static_cast<float>(L.width/L.height);
}

} //tr
