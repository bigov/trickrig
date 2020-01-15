#include "gui.hpp"
#include <thread>

namespace tr {

///
/// \brief gui::gui
///
gui::gui(wglfw* pMW, wglfw* pTW)
{
  MainWindow = pMW;
  MainWindow->gl_context_set_current();

  layout_set(cfg::WinLayout);
  MainWindow->set_window(Layout.width, Layout.height, MIN_GUI_WIDTH, MIN_GUI_HEIGHT, Layout.left, Layout.top);
  Space = std::make_unique<space>(pMW, pTW);
  FontMap1_len = static_cast<uint>(FontMap1.length());
  TimeStart = std::chrono::system_clock::now();

  // Составить список карт в каталоге пользователя
  auto MapsDirs = dirs_list(cfg::user_dir()); // список директорий с картами
  for(auto &P: MapsDirs) { Maps.push_back(map(P, cfg::map_name(P))); }

  // настройка текстуры для GUI
  glActiveTexture(GL_TEXTURE2);
  //glActiveTexture(GL_TEXTURE3);

  glGenTextures(1, &texture_gui);
  glBindTexture(GL_TEXTURE_2D, texture_gui);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  glBindTexture(GL_TEXTURE_2D, 0);

  ImgGUI.resize(Layout.width, Layout.height);

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

  glUniform1i(Program2d->uniform("texture_1"), 1);  // GL_TEXTURE1
  glUniform1i(Program2d->uniform("texture_2"), 2);  // GL_TEXTURE2

  Program2d->unuse();
  glBindVertexArray(0);
}


///
/// \brief gui::~gui
///
gui::~gui(void)
{
  cfg::save(Layout); // Сохранение положения окна
}


///
/// \brief Создание заголовка экрана
/// \param title
///
void gui::title(const std::string &title)
{
  img label{ ImgGUI.w_summ - 4, Font18s.h_cell * 2 - 4, color_title};

  ulong x = ImgGUI.w_summ/2 - utf8_size(title) * Font18s.w_cell / 2;
  textstring_place(Font18s, title, label, x, Font18s.h_cell/2);
  label.copy(0, 0, ImgGUI, 2, 2);
}


///
/// \brief Нарисовать поле ввода текстовой строки
/// \param _Fn - шрифт
///
/// \details Рисует указанным шрифтом, в фиксированной позиции, на всю
///  ширину экрана
///
void gui::input_text_line(const img &Font)
{
  px color = {0xF0, 0xF0, 0xF0, 0xFF};
  uint row_width = ImgGUI.w_summ - Font.w_cell * 2;
  uint row_height = Font.h_cell * 2;
  img RowInput{ row_width, row_height, color };

  // добавить текст, введенный пользователем
  uint y = (row_height - Font.h_cell)/2;
  textstring_place(Font, StringBuffer, RowInput, Font.w_cell, y);
  cursor_text_row(Font, RowInput, utf8_size(StringBuffer));

  // скопировать на экран изображение поля ввода с добавленым текстом
  auto x = (ImgGUI.w_summ - RowInput.w_summ) / 2;
  y = ImgGUI.h_summ / 2 - 2 * BUTTTON_HEIGHT;
  RowInput.copy(0, 0, ImgGUI, x, y);
}


///
/// \brief gui::add_text_cursor
/// \param _Fn       шрифт ввода
/// \param _Dst      строка ввода
/// \param position  номер позиции курсора в строке ввода
/// \details Формирование курсора ввода, моргающего с интервалом в пол-секунды
///
void gui::cursor_text_row(const img &_Fn, img &_Dst, size_t position)
{
  px c = {0x11, 0xDD, 0x00, 0xFF};
  auto tm = std::chrono::duration_cast<std::chrono::milliseconds>
      ( std::chrono::system_clock::now()-TimeStart ).count();

  auto tc = trunc(tm/1000) * 1000;
  if(tm - tc > 500) c.a = 0xFF;
  else c.a = 0x00;

  img Cursor {3, _Fn.h_cell, c};
  Cursor.copy(0, 0, _Dst, _Fn.w_cell * (position + 1) + 1,
              (_Dst.h_cell - _Fn.h_cell) / 2 );
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
void gui::row_text(size_t id, uint x, uint y, uint w, uint h, const std::string &text)
{
  px
    normal = color_title,                   // обычный цвет
    over = { 0xFF, 0xFF, 0xFF, 0xFF },      // цвет строки когда курсор над строкой
    selected = { 0xDD, 0xFF, 0xDD, 0xFF };  // цвет выбранной строки

  if(row_selected == id) // если строка уже выбрана, то ее цвет всегда "selected"
  {                       // не зависимо от положения на экране указателя мыши
    normal = selected;
    over = selected;
  }
  px bg_color = normal;

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

  img Row { w, h, bg_color };
  textstring_place(Font18n, text, Row, Font18n.w_cell/2, 6);
  Row.copy(0, 0, ImgGUI, x, y);
}


///
/// \brief gui::draw_list_select
/// \details Отображение списка выбора
///
void gui::select_list(uint lx, uint ly, uint lw, uint lh)
{
  img ListImg {lw, lh, {0xDD, 0xDD, 0xDD, 0xFF}};             // изображение списка
  ListImg.copy(0, 0, ImgGUI, lx, ly);

  uint rh = Font18n.h_cell * 1.5f;     // высота строки
  uint rw = lw - 4;                    // ширина строки
  uint max_rows = (lh - 4) / (rh + 2); // число строк, которое может поместиться в списке

  uint id = 0;
  for(auto &TheMap: Maps)
  {
    row_text(id + 1, lx + 2, ly + id * (rh + 2) + 2, rw, rh, TheMap.Name);
    if(++id > max_rows) break;
  }
}


///
/// \brief gui::cancel Отмена текущего режима
///
void gui::cancel(void)
{
  key    = EMPTY;
  action = EMPTY;

  switch (GuiMode)
  {
    case GUI_3D_MODE:
      cfg::map_view_save(Space->ViewFrom, Space->look_dir);
      GuiMode = GUI_MENU_LSELECT;
      Cursor3D[2] = 0.0f;                      // Спрятать прицел
      MainWindow->cursor_restore();             // Включить указатель мыши
      MainWindow->set_cursor_observer(*this);   // переключить обработчик смещения курсора
      MainWindow->set_button_observer(*this);   // обработчик кнопок мыши
      MainWindow->set_keyboard_observer(*this); // и клавиатуры
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
}


///
/// \brief gui::create_map
/// \details создается новая карта и сразу открывается
///
void gui::create_map(void)
{
  auto MapDir = cfg::create_map(StringBuffer);
  Maps.push_back(map(MapDir, StringBuffer));
  row_selected = Maps.size();     // выбрать номер карты
  button_click(BTN_OPEN);         // открыть
}


///
/// \brief gui::remove_map
///
void gui::remove_map(void)
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
      info("Can't remove the map: " + map_dir + "\n" );
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
void gui::button_click(ELEMENT_ID id)
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
      ImgGUI.fill({0xD0, 0xDD, 0xEE, 0xFF});      // заливка окна фоном
      {
        auto& Font = Font18s;
        char message[] = "ЗАГРУЗКА ДАННЫХ ...";     // В сообщении 19 символов
        textstring_place( Font, message, ImgGUI,
                        ( Layout.width - 19 * Font.w_cell)/2,
                        ( Layout.height - Font.h_cell)    /2 );
      }
      glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA,       // скопировать в графический буфер
                   static_cast<GLint>(Layout.width),
                   static_cast<GLint>(Layout.height),
                   0, GL_RGBA, GL_UNSIGNED_BYTE, ImgGUI.uchar_t());
      screen_render();     // Вывести на экран сообщение о загрузке
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
void gui::button_make_body(img &D, BUTTON_STATE s)
{
  double step = static_cast<double>(D.h_summ-3) / 256.0 * 7.0; // градации цвета

  // Используемые цвета
  px line_0  { 0xB6, 0xB6, 0xB3, 0xFF }; // верх и боковые
  px line_f  { 0x91, 0x91, 0x8C, 0xFF }; // нижняя
  px line_1;                             // блик (вторая линия)
  px line_bg;                            // фоновый цвет

  // Настройка цветовых значений
  switch (s) {
    case ST_PRESSED:
      line_bg = { 0xDE, 0xDE, 0xDE, 0xFF };
      line_1  = line_bg;
      step = 0;
      break;
    case ST_OVER:
      line_1  = { 0xFF, 0xFF, 0xFF, 0xFF };
      line_bg = { 0xF6, 0xF6, 0xF6, 0xFF };
      break;
    case ST_OFF:
      line_bg = { 0xD9, 0xD9, 0xD9, 0xFF };
      line_1  = line_bg;
      step = 0;
      break;
    case ST_NORMAL:
      line_1  = { 0xFA, 0xFA, 0xFA, 0xFF };
      line_bg = { 0xE7, 0xE7, 0xE6, 0xFF };
      break;
  }

  // верхняя линия
  size_t i = 0;
  size_t max = D.w_summ;
  while(i < max) D.Data[i++] = line_0;

  // вторая линия
  max += D.w_summ;
  while(i < max) D.Data[i++] = line_1;

  // основной фон
  int S = 0;         // коэффициент построчного уменьшения яркости
  uint np = 0;       // счетчик значений
  double nr = 0.0;   // счетчик строк

  max += D.w_summ * (D.h_summ - 3);
  while (i < max)
  {
    D.Data[i++] = { static_cast<int>(line_bg.r) - S,
                    static_cast<int>(line_bg.g) - S,
                    static_cast<int>(line_bg.b)  - S,
                    static_cast<int>(line_bg.a) };
    np++;
    if(np >= D.w_summ)
    {
      np = 0;
      nr += 1.0;
      S = static_cast<uchar>(nr * step);
    }

  }

  // нижняя линия
  max += D.w_summ;
  while(i < max)
  {
    D.Data[i++] = line_f;
  }

  // боковинки
  i = 0;
  while(i < max)
  {
    D.Data[i++] = line_f;
    i += D.w_summ - 2;
    D.Data[i++] = line_f;
  }
}


///
/// \brief Формирование кнопки
///
/// Вначале формируется отдельное изображение кнопки, потом оно копируется
/// в указанное координатами (x,y) место окна.
///
void gui::button(ELEMENT_ID btn_id, ulong x, ulong y,
                     const std::string &Name, bool button_is_active)
{
  img Btn { BUTTTON_WIDTH, BUTTTON_HEIGHT };

  if(button_is_active)
  {
    // Если указатель находится над кнопкой
    if( mouse_x >= x && mouse_x <= x + BUTTTON_WIDTH &&
        mouse_y >= y && mouse_y <= y + BUTTTON_HEIGHT)
    {
      element_over = btn_id;

      // и если кнопку "кликнули" указателем мыши
      if(mouse_left == PRESS)
      {
        button_make_body(Btn, ST_PRESSED);
      }
      else
      {
        button_make_body(Btn, ST_OVER);
      }
    }
    else
    {
      button_make_body(Btn, ST_NORMAL);
    }
  }
  else
  {
    button_make_body(Btn, ST_OFF);
  }

  auto t_width = Font18s.w_cell * utf8_size(Name);
  auto t_height = Font18s.h_cell;

  if(button_is_active)
  {
    textstring_place(Font18s, Name, Btn, BUTTTON_WIDTH/2 - t_width/2,
           BUTTTON_HEIGHT/2 - t_height/2);
  }
  else
  {
    textstring_place(Font18l, Name, Btn, BUTTTON_WIDTH/2 - t_width/2,
           BUTTTON_HEIGHT/2 - t_height/2);
  }
  Btn.copy(0, 0, ImgGUI, x, y);
}


///
/// \brief Начальный экран приложения
///
void gui::menu_start(void)
{
  ImgGUI.fill(bg);
  title("Trick Rig");

  uint x = Layout.width/2 - BUTTTON_WIDTH/2;   // X координата кнопки
  uint y = Layout.height/2 - BUTTTON_HEIGHT/2;  // Y координата кнопки
  button(BTN_CONFIG, x, y, "Настроить");

  y -= 1.5 * BUTTTON_HEIGHT;
  button(BTN_LOCATION, x, y, "Старт");

  y += 3 * BUTTTON_HEIGHT;
  button(BTN_CANCEL, x, y, "Закрыть");
}


///
/// \brief Окно выбора района
///
void gui::menu_map_select(void)
{
  ImgGUI.fill(bg);
  title("ВЫБОР КАРТЫ");

  // Список фиксированой ширины и один ряд кнопок размещается в центре окна на
  // расстоянии 1/8 высоты окна сверху и снизу, и 1/8 ширины окна по бокам.
  // Расстояние между списком и кнопками равно половине высоты кнопки.

  uint y = Layout.height/8; // отступ сверху (и снизу)
  uint list_h = Layout.height - y * 2 - BUTTTON_HEIGHT * 1.5f;
  uint list_w = MIN_GUI_WIDTH - 4;
  uint x = (Layout.width - list_w)/2;  // отступ слева (и справа)

  select_list(x, y, list_w, list_h);

  x = ImgGUI.w_summ / 2 + 8;
  y = y + list_h + BUTTTON_HEIGHT/2;

  button(BTN_CANCEL, x, y, "Отмена");
  button(BTN_MAP_DELETE, x + BUTTTON_WIDTH + 16, y, "Удалить", row_selected > 0);
  button(BTN_OPEN, x - (BUTTTON_WIDTH + 16), y, "Открыть", row_selected > 0);
  button(BTN_CREATE, x - (BUTTTON_WIDTH + 16)*2, y, "Создать");

}


///
/// \brief Экран ввода названия для создания новой карты
///
/// \details Предлагается строка ввода названия. При нажатии кнопки
/// BTN_ENTER_NAME введенный в строке текст будет использован для создания
/// нового файла хранения данных 3D пространства района.
///
void gui::menu_map_create(void)
{
  ImgGUI.fill(bg);
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
  auto x = ImgGUI.w_summ / 2 - static_cast<ulong>(BUTTTON_WIDTH * 1.25);
  auto y = ImgGUI.h_summ / 2;
  button(BTN_ENTER_NAME, x, y, "OK", StringBuffer.length() > 0);

  x += BUTTTON_WIDTH * 1.5;  // X координата кнопки
  button(BTN_LOCATION, x, y, "Отмена");
}


///
/// \brief gui::menu_config
///
void gui::menu_config(void)
{
  ImgGUI.fill(bg);
  title("НАСТРОЙКИ");

  int x = ImgGUI.w_summ / 2 - static_cast<ulong>(BUTTTON_WIDTH/2);
  int y = ImgGUI.h_summ / 2;
  button(BTN_CANCEL, x, y, "Отмена");
}


///
/// \brief gui::draw_gui_menu
///
void gui::menu_build(void)
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

  // каждый кадр на текстуре окна обновляем изображение меню
  //glActiveTexture(GL_TEXTURE3);
  glBindTexture(GL_TEXTURE_2D, texture_gui);
  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA,
               static_cast<GLint>(Layout.width),
               static_cast<GLint>(Layout.height),
               0, GL_RGBA, GL_UNSIGNED_BYTE, ImgGUI.uchar_t());

  if((mouse_left == RELEASE) && (element_over != NONE))
  {
    button_click(element_over);
    mouse_left = EMPTY;
    action = EMPTY;         // сбросить флаг действия
  }

  if(element_over == NONE)
  {
    mouse_left = EMPTY;
    //AppWin.action = -1; // флаг действия потребуется при вводе названия карты
  }

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
void gui::show(void)
{
  MainWindow->set_char_observer(*this);
  MainWindow->set_error_observer(*this);     // отслеживание ошибок
  MainWindow->set_cursor_observer(*this);    // курсор мыши в окне
  MainWindow->set_button_observer(*this);    // кнопки мыши
  MainWindow->set_keyboard_observer(*this);  // клавиши клавиатуры
  MainWindow->set_position_observer(*this);  // положение окна
  MainWindow->add_size_observer(*this);      // размер окна
  MainWindow->set_close_observer(*this);     // закрытие окна
  MainWindow->set_focuslost_observer(*this); // потеря окном фокуса ввода

  while(is_open)
  {
    Space->render(); // рендер 3D сцены
    menu_build();    // рендер GUI меню
    screen_render(); // прорисовка окна приложения
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
void gui::screen_render(void)
{
  glBindVertexArray(vao_quad_id);
  glDisable(GL_DEPTH_TEST);
  Program2d->use();
  VboAccess.lock();
  Program2d->set_uniform("Cursor", Cursor3D);
  glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
  VboAccess.unlock();
  Program2d->unuse();

  // переключить буфер рендера
  VboAccess.lock();
  MainWindow->swap_buffers();
  VboAccess.unlock();

#ifndef NDEBUG
  if(glGetError() != GL_NO_ERROR) info("ERROR in the function 'gui::render_screen'.");
#endif

}

///
/// \brief gui::window_pos_event
/// \param left
/// \param top
///
void gui::reposition_event(int _left, int _top)
{
  Layout.left = static_cast<uint>(_left);
  Layout.top = static_cast<uint>(_top);
}


///
/// \brief gui::resize_event
/// \param width
/// \param height
///
void gui::resize_event(int w, int h)
{
  assert(w >= 0);
  assert(h >= 0);

  Layout.width  = static_cast<uint>(w);
  Layout.height = static_cast<uint>(h);

  // пересчет позции координат прицела (центр окна)
  Cursor3D.x = static_cast<float>(w/2);
  Cursor3D.y = static_cast<float>(h/2);

  // пересчет размеров изображения GUI
  ImgGUI.resize(w,h);
}


///
/// \brief gui::character_event
/// \param ch
///
void gui::character_event(uint ch)
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
/// \brief gui::cursor_position_event
/// \param x
/// \param y
///
void gui::cursor_event(double x, double y)
{
  mouse_x = x;
  mouse_y = y;
}


///
/// \brief win_data::close_event
///
void gui::close_event(void)
{
  is_open = false;
}


///
/// \brief gui::error_event
/// \param message
///
void gui::error_event(const char* message)
{
  info(std::string(message));
}


///
/// \brief gui::mouse_event
/// \param _button
/// \param _action
/// \param _mods
///
void gui::mouse_event(int _button, int _action, int _mods)
{
  mods   = _mods;
  action = _action;

  if (_button == MOUSE_BUTTON_LEFT) mouse_left = _action;
  else mouse_left = EMPTY;
}


///
/// \brief gui::keyboard_event
/// \param key
/// \param scancode
/// \param action
/// \param mods
///
void gui::keyboard_event(int _key, int _scancode, int _action, int _mods)
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
void gui::focus_lost_event()
{
  if (GUI_3D_MODE == GuiMode)
  {
     cfg::map_view_save(Space->ViewFrom, Space->look_dir);
     GuiMode = GUI_MENU_LSELECT;
     Cursor3D[2] = 0.0f;                      // Спрятать прицел
     MainWindow->cursor_restore();            // Включить указатель мыши
     MainWindow->set_cursor_observer(*this);  // переключить обработчик смещения курсора
     MainWindow->set_button_observer(*this);  // обработчик кнопок мыши
  }
}


///
/// \brief gui::set_location
/// \param width
/// \param height
/// \param left
/// \param top
///
void gui::layout_set(const layout &L)
{
  Layout = L;
  Cursor3D.x = static_cast<float>(L.width/2);
  Cursor3D.y = static_cast<float>(L.height/2);
  aspect  = static_cast<float>(L.width/L.height);
}

} //tr
