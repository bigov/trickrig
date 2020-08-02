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

std::unique_ptr<gui> app::AppGUI = nullptr;

app::MENU_MODES app::MenuMode = SCREEN_START;    // режим окна приложения

std::unique_ptr<glsl> Program2d = nullptr;            // построение 2D элементов

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

  Layout = cfg::WinLayout;
  aspect  = static_cast<float>(Layout.width/Layout.height);
  GLContext->set_window(Layout.width, Layout.height, MIN_GUI_WIDTH, MIN_GUI_HEIGHT, Layout.left, Layout.top);

  init_prog_2d();      // Шейдерная программа для построения 2D элементов пользовательского интерфейса
  load_font_texture(); // Загрузчик текстуры со шрифтом

  AppGUI = std::make_unique<gui>(GLContext);

  TimeStart = std::chrono::system_clock::now();

  // Составить список карт в каталоге пользователя
  auto MapsDirs = dirs_list(cfg::user_dir()); // список директорий с картами
  for(auto &P: MapsDirs) { Maps.push_back(map(P, cfg::map_name(P))); }

  glActiveTexture(GL_TEXTURE2);     // текстура для GUI (меню приложения)
  glGenTextures(1, &texture_gui);
  glBindTexture(GL_TEXTURE_2D, texture_gui);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
}


///
/// \brief gui::~gui
///
app::~app(void)
{
  cfg::save(Layout); // Сохранение положения окна
}


///
/// \brief gui::add_text_cursor
/// \param _Fn       шрифт ввода
/// \param _Dst      строка ввода
/// \param position  номер позиции курсора в строке ввода
/// \details Формирование курсора ввода, моргающего с интервалом в пол-секунды
///
//void app::cursor_text_row(const atlas &_Fn, image &_Dst, size_t position)
void app::cursor_text_row(const atlas&, image&, size_t)
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

  while(AppGUI->open) AppGUI->render();
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

} //tr
