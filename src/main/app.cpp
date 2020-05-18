#include "app.hpp"

namespace tr {

#define MAX_VERTEX_BUFFER 512 * 1024
#define MAX_ELEMENT_BUFFER 128 * 1024


menu_screen app::MainMenu {};          // Начальное меню приложения
bool        app::is_open = true;       // состояние окна
GLuint      app::texture_gui = 0;      // id тектуры HUD
layout      app::Layout {};            // размеры и положение окна
std::vector<map> app::Maps {};    // список карт
uchar_color app::color_title {0xFF, 0xFF, 0xDD, 0xFF}; // фон заголовка
size_t      app::row_selected = 0;     // какая строка выбрана
double      app::mouse_x = 0.0;        // позиция указателя относительно левой границы
double      app::mouse_y = 0.0;        // позиция указателя относительно верхней границы
int         app::mouse_left = EMPTY;   // нажатие на левую кнопку мыши
ELEMENT_ID  app::element_over = NONE;  // Над какой GIU кнопкой курсор


///
/// \brief gui::gui
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
  Space = std::make_unique<space>(GLContext);
  FontMap1_len = static_cast<uint>(FontMap1.length());
  TimeStart = std::chrono::system_clock::now();

  // Составить список карт в каталоге пользователя
  auto MapsDirs = dirs_list(cfg::user_dir()); // список директорий с картами
  for(auto &P: MapsDirs) { Maps.push_back(map(P, cfg::map_name(P))); }

  // настройка текстуры для GUI
  glActiveTexture(GL_TEXTURE2);

  glGenTextures(1, &texture_gui);
  glBindTexture(GL_TEXTURE_2D, texture_gui);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  glBindTexture(GL_TEXTURE_2D, 0);

  /// Инициализация GLSL программы обработки текстуры фреймбуфера.
  ///
  /// Текстура фрейм-буфера за счет измения порядка следования координат
  /// вершин с 1-2-3-4 на 3-4-1-2 перевернута - верх и низ в сцене
  /// меняются местами. Благодаря этому, нулевой координатой (0,0) окна
  /// становится более привычный верхний-левый угол, и загруженные из файла
  /// изображения текстур применяются без дополнительного переворота.

  GLfloat Position[8] = { // XY координаты вершин
    -1.f,-1.f,
     1.f,-1.f,
    -1.f, 1.f,
     1.f, 1.f
  };

  GLfloat Texcoord[8] = { // UV координаты текстуры
    0.f, 1.f, //3
    1.f, 1.f, //4
    0.f, 0.f, //1
    1.f, 0.f, //2
  };

  glGenVertexArrays(1, &vao_quad_id);
  glBindVertexArray(vao_quad_id);

  std::list<std::pair<GLenum, std::string>> Shaders {};
  Shaders.push_back({ GL_VERTEX_SHADER, cfg::app_key(SHADER_VERT_SCREEN) });
  Shaders.push_back({ GL_FRAGMENT_SHADER, cfg::app_key(SHADER_FRAG_SCREEN) });

  Program2d = std::make_unique<glsl>(Shaders);
  Program2d->use();

  vbo VboPosition { GL_ARRAY_BUFFER };

  VboPosition.allocate( sizeof(Position), Position );
  VboPosition.attrib( Program2d->attrib("position"),
      2, GL_FLOAT, GL_FALSE, 0, 0);

  vbo VboTexcoord { GL_ARRAY_BUFFER };

  VboTexcoord.allocate( sizeof(Texcoord), Texcoord );
  VboTexcoord.attrib( Program2d->attrib("texcoord"),
      2, GL_FLOAT, GL_FALSE, 0, 0);

  glUniform1i(Program2d->uniform("texture_1"), 1); // Текстурный блок фрейм-буфера - glActiveTecsture(GL_TEXTURE1)
  glUniform1i(Program2d->uniform("texture_2"), 2); // В зависимости от режима HUD или GUI - glActiveTecsture(GL_TEXTURE2)

  Program2d->unuse();
  glBindVertexArray(0);
}


///
/// \brief gui::~gui
///
app::~app(void)
{
  cfg::save(Layout); // Сохранение положения окна
}


///
/// \brief Создание заголовка экрана
/// \param title
///
void app::title(const std::string &title)
{
  image Box{ MainMenu.get_width() - 4, 24, color_title};
  label Text {title, 24, FONT_BOLD};
  Box.paint_over((Box.get_width() - Text.get_width())/2,
                 (Box.get_height() - Text.get_height())/2,
                  Text);
  Box.put(MainMenu, 2, 2);
}


///
/// \brief Нарисовать поле ввода текстовой строки
/// \param _Fn - шрифт
///
/// \details Рисует указанным шрифтом, в фиксированной позиции, на всю
///  ширину экрана
///
void app::input_text_line(const texture &Font)
{
  uchar_color color = {0xF0, 0xF0, 0xF0, 0xFF};
  uint row_width = MainMenu.get_width() - Font.get_cell_width() * 2;
  uint row_height = Font.get_cell_height() * 2;
  image RowInput{ row_width, row_height, color };

  // добавить текст, введенный пользователем
  uint y = (row_height - Font.get_cell_height())/2;
  textstring_place(Font, StringBuffer, RowInput, Font.get_cell_width(), y);
  cursor_text_row(Font, RowInput, utf8_size(StringBuffer));

  // скопировать на экран изображение поля ввода с добавленым текстом
  auto x = (MainMenu.get_width() - RowInput.get_width()) / 2;
  y = MainMenu.get_height() / 2 - 2 * BUTTTON_HEIGHT;
  RowInput.put(MainMenu, x, y);
}


///
/// \brief gui::add_text_cursor
/// \param _Fn       шрифт ввода
/// \param _Dst      строка ввода
/// \param position  номер позиции курсора в строке ввода
/// \details Формирование курсора ввода, моргающего с интервалом в пол-секунды
///
void app::cursor_text_row(const texture &_Fn, image &_Dst, size_t position)
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
void app::row_text(size_t id, uint x, uint y, uint w, uint h, const std::string &text)
{
  uchar_color
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
}


///
/// \brief gui::draw_list_select
/// \details Отображение списка выбора
///
void app::select_list(uint lx, uint ly, uint lw, uint lh)
{
  image ListImg {lw, lh, {0xDD, 0xDD, 0xDD, 0xFF}};             // изображение списка
  ListImg.put(MainMenu, lx, ly);

  uint rh = Font18n.get_cell_height() * 1.5f;     // высота строки
  uint rw = lw - 4;                               // ширина строки
  uint max_rows = (lh - 4) / (rh + 2);            // число строк, которое может поместиться в списке

  uint id = 0;
  for(auto& Item: Maps)
  {
    row_text(id + 1, lx + 2, ly + id * (rh + 2) + 2, rw, rh, Item.Name);
    if(++id > max_rows) break;
  }
}


///
/// \brief gui::cancel Отмена текущего режима
///
void app::cancel(void)
{
  key    = EMPTY;
  action = EMPTY;

  switch (GuiMode)
  {
    case GUI_3D_MODE:
      cfg::map_view_save(Space->ViewFrom, Space->look_dir);
      GuiMode = GUI_MENU_LSELECT;
      Cursor3D[2] = 0.0f;                      // Спрятать прицел
      GLContext->cursor_restore();             // Включить указатель мыши
      GLContext->set_cursor_observer(*this);   // переключить обработчик смещения курсора
      GLContext->set_button_observer(*this);   // обработчик кнопок мыши
      GLContext->set_keyboard_observer(*this); // и клавиатуры
      break;
    case GUI_MENU_LSELECT:
      GuiMode = GUI_MENU_START;
      break;
    case GUI_MENU_CREATE:
      GuiMode = GUI_MENU_LSELECT;
      break;
    case GUI_MENU_CONFIG:
      GuiMode = GUI_MENU_START;
      break;
    case GUI_MENU_START:
      is_open = false;
      break;
  }

  //@@@
  menu_build();
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
  button_click(BTN_OPEN);         // открыть
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
/// \brief gui::button_click - Обработчик нажатия клавиш GUI интерфейса
///
void app::button_click(ELEMENT_ID id)
{
  static ELEMENT_ID double_id = NONE;

  text_mode = false; // Во всех режимах, кроме GUI_MENU_CREATE,
                     // строка ввода отключена

  if(GuiMode == GUI_3D_MODE) return;

  if(id == ROW_MAP_NAME)
  { // В списке карт первый клик выбирает карту, второй открывает.
    static size_t row_id = 0;
    if(double_id == id && row_id == row_selected) id = BTN_OPEN;
    row_id = row_selected;
    double_id = id;
  }

  switch(id)
  {
    case BTN_OPEN:
      cfg::map_view_load(Maps[row_selected - 1].Folder, Space->ViewFrom, Space->look_dir);
      GuiMode = GUI_3D_MODE;
      Space->enable();     // Загрузка занимает некоторое время ...
      Cursor3D[2] = 4.0f;  // Активировать прицел
      break;
    case BTN_CONFIG:
      GuiMode = GUI_MENU_CONFIG;
      break;
    case BTN_LOCATION:
      GuiMode = GUI_MENU_LSELECT;
      break;
    case BTN_CREATE:
      text_mode = true;
      StringBuffer.clear();
      GuiMode = GUI_MENU_CREATE;
      break;
    case BTN_ENTER_NAME:
      create_map();
      break;
    case BTN_CANCEL:
      cancel();
      break;
    case BTN_MAP_DELETE:
      remove_map();
      break;
    default: break;
  }

  //@@@
  menu_build();

}


///
/// \brief Построение картинки кнопки
/// \param w
/// \param h
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
void app::button_make_body(image &D, BTN_STATE s)
{
  double step = static_cast<double>(D.get_height() - 3) / 256.0 * 7.0; // градации цвета

  // Используемые цвета
  uchar_color line_0  { 0xB6, 0xB6, 0xB3, 0xFF }; // верх и боковые
  uchar_color line_f  { 0x91, 0x91, 0x8C, 0xFF }; // нижняя
  uchar_color line_1;                             // блик (вторая линия)
  uchar_color line_bg;                            // фоновый цвет

  // Настройка цветовых значений
  switch (s) {
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
  uchar_color* DData = D.color_data();
  size_t i = 0;
  size_t max = D.get_width();
  while(i < max) DData[i++] = line_0;

  // вторая линия
  max += D.get_width();
  while(i < max) DData[i++] = line_1;

  // основной фон
  uchar S = 0;         // коэффициент построчного уменьшения яркости
  uint np = 0;       // счетчик значений
  double nr = 0.0;   // счетчик строк

  max += D.get_width() * (D.get_height() - 3);
  while (i < max)
  {
    DData[i++] = { static_cast<uchar>(line_bg.r - S),
                    static_cast<uchar>(line_bg.g - S),
                    static_cast<uchar>(line_bg.b - S),
                    static_cast<uchar>(line_bg.a) };
    np++;
    if(np >= D.get_width())
    {
      np = 0;
      nr += 1.0;
      S = static_cast<uchar>(nr * step);
    }

  }

  // нижняя линия
  max += D.get_width();
  while(i < max)
  {
    DData[i++] = line_f;
  }

  // боковинки
  i = 0;
  while(i < max)
  {
    DData[i++] = line_f;
    i += D.get_width() - 2;
    DData[i++] = line_f;
  }
}


///
/// \brief Формирование кнопки
///
/// Вначале формируется отдельное изображение кнопки, потом оно копируется
/// в указанное координатами (x,y) место окна.
///
void app::btn(ELEMENT_ID btn_id, ulong x, ulong y,
                     const std::string &Name, bool enable)
{
  image BtnImage { BUTTTON_WIDTH, BUTTTON_HEIGHT };

  uchar_color FontColor {24, 24, 24, 0};
  if(enable)
  {
    // Если указатель находится над кнопкой
    if( mouse_x >= x && mouse_x <= x + BUTTTON_WIDTH &&
        mouse_y >= y && mouse_y <= y + BUTTTON_HEIGHT)
    {
      element_over = btn_id;

      // и если кнопку "кликнули" указателем мыши
      if(mouse_left == PRESS)
      {
        button_make_body(BtnImage, BTN_PRESSED);
      }
      else
      {
        button_make_body(BtnImage, BTN_OVER);
      }
    }
    else
    {
      button_make_body(BtnImage, BTN_NORMAL);
    }
  }
  else
  {
    button_make_body(BtnImage, BTN_DISABLE);
    FontColor = { 0xFF, 0xFF, 0xFF, 0};
  }

  label Text(Name, 26, FONT_NORMAL, {FontColor.r, FontColor.g, FontColor.b, FontColor.a } );

  BtnImage.paint_over((BtnImage.get_width() - Text.get_width())/2,
                 (BtnImage.get_height() - Text.get_height())/2, Text);

  MainMenu.paint_over(x, y, BtnImage);
}


///
/// \brief app::app_close
///
void app::app_close(void)
{
  is_open = false;
}

///
/// \brief Начальный экран приложения
///
void app::menu_start(void)
{
  MainMenu.init(Layout.width, Layout.height, "New TrickRig");

  uint x = Layout.width/2 - BUTTTON_WIDTH/2;   // X координата кнопки
  uint y = Layout.height/2 - BUTTTON_HEIGHT/2;  // Y координата кнопки

  MainMenu.button_add(x, y, "Настроить", menu_config); //BTN_CONFIG

  y -= 1.2 * BUTTTON_HEIGHT;
  MainMenu.button_add(x, y, "Старт", menu_map_select); // BTN_LOCATION

  y += 2.4 * BUTTTON_HEIGHT;
  MainMenu.button_add(x, y, "Закрыть", app_close); // BTN_CANCEL

  update_texture_gui();
}


///
/// \brief Окно выбора района
///
void app::menu_map_select(void)
{
  MainMenu.init(Layout.width, Layout.height, "Выбор карты");

  // Список фиксированой ширины и один ряд кнопок размещается в центре окна на
  // расстоянии 1/8 высоты окна сверху и снизу, и 1/8 ширины окна по бокам.
  // Расстояние между списком и кнопками равно половине высоты кнопки.

  uint y = Layout.height/8; // отступ сверху (и снизу)
  uint list_h = Layout.height - y * 2 - BUTTTON_HEIGHT * 1.5f;
  uint list_w = MIN_GUI_WIDTH - 4;
  uint x = (Layout.width - list_w)/2;  // отступ слева (и справа)

  select_list(x, y, list_w, list_h);

  x = MainMenu.get_width() / 2 + 8;
  y = y + list_h + BUTTTON_HEIGHT/2;

  MainMenu.button_add(x, y, "Отмена", menu_start);
  MainMenu.button_add(x + BUTTTON_WIDTH + 16, y, "Удалить");
  MainMenu.button_add(x - (BUTTTON_WIDTH + 16), y, "Открыть");
  MainMenu.button_add(x - (BUTTTON_WIDTH + 16)*2, y, "Создать");
  /*
  btn(BTN_CANCEL, x, y, "Отмена");
  btn(BTN_MAP_DELETE, x + BUTTTON_WIDTH + 16, y, "Удалить", row_selected > 0);
  btn(BTN_OPEN, x - (BUTTTON_WIDTH + 16), y, "Открыть", row_selected > 0);
  btn(BTN_CREATE, x - (BUTTTON_WIDTH + 16)*2, y, "Создать");
  */
}


///
/// \brief Экран ввода названия для создания новой карты
///
/// \details Предлагается строка ввода названия. При нажатии кнопки
/// BTN_ENTER_NAME введенный в строке текст будет использован для создания
/// нового файла хранения данных 3D пространства района.
///
void app::menu_map_create(void)
{
  MainMenu.fill(bgColor);
  title("ВВЕДИТЕ НАЗВАНИЕ");

  if((key == KEY_BACKSPACE) && // Удаление введенных символов
    ((action == PRESS)||(action == REPEAT)))
  {
    if (StringBuffer.length() == 0) return;
    if(char_type(StringBuffer[StringBuffer.size()-1]) != SINGLE)
    { StringBuffer.pop_back(); } // если это UTF-8, то удаляем два байта
    StringBuffer.pop_back();
    std::this_thread::sleep_for(std::chrono::milliseconds(75));
  }

  // строка ввода текста
  input_text_line(Font18n);

  // две кнопки
  auto x = MainMenu.get_width() / 2 - static_cast<ulong>(BUTTTON_WIDTH * 1.25);
  auto y = MainMenu.get_height() / 2;
  btn(BTN_ENTER_NAME, x, y, "OK", StringBuffer.length() > 0);

  x += BUTTTON_WIDTH * 1.5;  // X координата кнопки
  btn(BTN_LOCATION, x, y, "Отмена");
}


///
/// \brief gui::menu_config
///
void app::menu_config(void)
{
  MainMenu.init(Layout.width, Layout.height,"Настройка");

  int x = MainMenu.get_width() / 2 - static_cast<ulong>(BUTTTON_WIDTH / 2);
  int y = MainMenu.get_height() / 2;
  MainMenu.button_add(x, y, "Отмена", menu_start);
  update_texture_gui();
}


///
/// \brief app::update_texture_gui
///
void app::update_texture_gui(void)
{
  glBindTexture(GL_TEXTURE_2D, texture_gui);
  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA,
               static_cast<GLint>(Layout.width),
               static_cast<GLint>(Layout.height),
               0, GL_RGBA, GL_UNSIGNED_BYTE, MainMenu.uchar_t());
}


///
/// \brief gui::menu_build
///
void app::menu_build(void)
{
  switch (GuiMode)
  {
    case GUI_3D_MODE:
      return;              // в режиме 3D-сцены меню не отображается
    case GUI_MENU_CONFIG:
      menu_config();
      break;
    case GUI_MENU_CREATE:
      menu_map_create();
      break;
    case GUI_MENU_LSELECT:
      menu_map_select();
      break;
    case GUI_MENU_START:
      menu_start();
      break;
    default: break;
   }

  // Обработка состояния мыши: клавиша отпущена над элементом меню
  if((mouse_left == RELEASE) && (element_over != NONE))
  {
    button_click(element_over);
    mouse_left = EMPTY;
    action = EMPTY;         // сбросить флаг действия
  }

  if(element_over == NONE) { mouse_left = EMPTY; }

  // При рисовании кнопки проверяются координаты указателя мыши. Если указатель
  // находится над кнопкой, то кнопка изображается другим цветом и ее ID
  // присваивается переменной "button_over"
  element_over = NONE;
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
  GLContext->set_char_observer(*this);
  GLContext->set_error_observer(*this);     // отслеживание ошибок
  GLContext->set_cursor_observer(*this);    // курсор мыши в окне
  GLContext->set_button_observer(*this);    // кнопки мыши
  GLContext->set_keyboard_observer(*this);  // клавиши клавиатуры
  GLContext->set_position_observer(*this);  // положение окна
  GLContext->add_size_observer(*this);      // размер окна
  GLContext->set_close_observer(*this);     // закрытие окна
  GLContext->set_focuslost_observer(*this); // потеря окном фокуса ввода

  menu_build();    // рендер GUI окна приложения

  while(is_open)
  {
    Space->render(); // рендер 3D сцены
    AppWin_render(); // прорисовка окна приложения

    // переключить буфер рендера
    vbo_mtx.lock();
    GLContext->swap_buffers();
    vbo_mtx.unlock();

  #ifndef NDEBUG
    if(glGetError() != GL_NO_ERROR) std::cerr << "ERROR in the function 'gui::render_screen'.";
  #endif

  }
}


///
/// \brief gui::screen_render
///
/// \details Рендер окна с текстурами фреймбуфера и GIU
/// Кадр сцены рендерится в изображение на (2D) "холсте" фреймбуфера,
/// после чего это изображение в виде текстуры накладывается на
/// прямоугольник окна. Курсор и дополнительные (HUD) элементы окна
/// изображаются как наложеные сверху дополнительные текстуры
///
void app::AppWin_render(void)
{
  if(GuiMode == GUI_3D_MODE) {
    glBindTexture(GL_TEXTURE_2D, Space->texture_hud);
  } else {
    //menu_build();    // рендер GUI меню
    glBindTexture(GL_TEXTURE_2D, texture_gui);
  }

  glBindVertexArray(vao_quad_id);
  glDisable(GL_DEPTH_TEST);
  Program2d->use();
  vbo_mtx.lock();
  Program2d->set_uniform("Cursor", Cursor3D);
  glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
  vbo_mtx.unlock();
  Program2d->unuse();
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
  MainMenu.resize(w,h);
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
  is_open = false;
}


///
/// \brief gui::error_event
/// \param message
///
void app::error_event(const char* message)
{
  std::cerr << message;
}


///
/// \brief gui::cursor_event
/// \param x
/// \param y
/// \details Изменение координат положения курсора мыши в окне
///
void app::cursor_event(double x, double y)
{
  //mouse_x = x;
  //mouse_y = y;
  if (MainMenu.cursor_event(x, y)) update_texture_gui();
}


///
/// \brief gui::mouse_event
/// \param _button
/// \param _action
/// \param _mods
///
void app::mouse_event(int _button, int _action, int _mods)
{
  //mods   = _mods;
  //action = _action;
  //if (_button == MOUSE_BUTTON_LEFT) mouse_left = _action;
  //else mouse_left = EMPTY;
  if (MainMenu.mouse_event(_button, _action, _mods)) update_texture_gui();
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
  if (GUI_3D_MODE == GuiMode) Space->keyboard_event(_key, _scancode, _action, _mods);
}


///
/// \brief gui::focus_event
/// \details Потеря окном фокуса в режиме рендера 3D сцены
/// переводит GUI в режим отображения меню
///
void app::focus_lost_event()
{
  if (GUI_3D_MODE == GuiMode)
  {
     cfg::map_view_save(Space->ViewFrom, Space->look_dir);
     GuiMode = GUI_MENU_LSELECT;
     Cursor3D[2] = 0.0f;                      // Спрятать прицел
     GLContext->cursor_restore();            // Включить указатель мыши
     GLContext->set_cursor_observer(*this);  // переключить обработчик смещения курсора
     GLContext->set_button_observer(*this);  // обработчик кнопок мыши
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
