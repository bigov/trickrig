#include "gui.hpp"
#include <thread>

namespace tr {

///
/// \brief gui::gui
///
gui::gui(wglfw* glContext)
{
  cfg::load();
  WinParams.layout_set(cfg::WinLayout);

  glContext->set_window(WinParams.Layout.width, WinParams.Layout.height,
                WinParams.minwidth, WinParams.minheight,
                WinParams.Layout.left, WinParams.Layout.top);

  MatProjection = glm::perspective(1.118f, WinParams.aspect, zNear, zFar);
  Space = std::make_unique<space>(glContext);
  FontMap1_len = static_cast<u_int>(FontMap1.length());
  TimeStart = std::chrono::system_clock::now();

  // Составить список карт в каталоге пользователя
  auto MapsDirs = dirs_list(cfg::user_dir()); // список директорий с картами
  for(auto &P: MapsDirs) { Maps.push_back(map(P, cfg::map_name(P))); }

  // настройка текстуры для HUD
  glActiveTexture(GL_TEXTURE2);

  glGenTextures(1, &gui_texture);
  glBindTexture(GL_TEXTURE_2D, gui_texture);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  glBindTexture(GL_TEXTURE_2D, 0);

  WinGui.resize(WinParams.Layout.width, WinParams.Layout.height);
  WinParams.pWinGui = &WinGui;

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

//  screenShaderProgram = std::make_unique<glsl>();

  glGenVertexArrays(1, &vao_quad_id);
  glBindVertexArray(vao_quad_id);

  screenShaderProgram.init();
  screenShaderProgram.attach_shaders(
        cfg::app_key(SHADER_VERT_SCREEN), cfg::app_key(SHADER_FRAG_SCREEN) );
  screenShaderProgram.use();

  vbo_base VboPosition { GL_ARRAY_BUFFER };

  VboPosition.allocate( sizeof(Position), Position );
  VboPosition.attrib( screenShaderProgram.attrib_location_get("position"),
      2, GL_FLOAT, GL_FALSE, 0, 0);

  vbo_base VboTexcoord { GL_ARRAY_BUFFER };

  VboTexcoord.allocate( sizeof(Texcoord), Texcoord );
  VboTexcoord.attrib( screenShaderProgram.attrib_location_get("texcoord"),
      2, GL_FLOAT, GL_FALSE, 0, 0);

  // GL_TEXTURE1
  glUniform1i(screenShaderProgram.uniform_location_get("texFramebuffer"), 1);
  // GL_TEXTURE2
  glUniform1i(screenShaderProgram.uniform_location_get("texHUD"), 2);

  screenShaderProgram.unuse();
  glBindVertexArray(0);
}


///
/// \brief gui::~gui
///
gui::~gui(void)
{
  screenShaderProgram.destroy();
  Space = nullptr;
  cfg::save(WinParams.Layout); // Сохранение положения окна
}


///
/// \brief Создание заголовка экрана
/// \param title
///
void gui::title(const std::string &title)
{
  img label{ WinGui.w_summ - 4, Font18s.h_cell * 2 - 4, color_title};

  u_long x = WinGui.w_summ/2 - utf8_size(title) * Font18s.w_cell / 2;
  textstring_place(Font18s, title, label, x, Font18s.h_cell/2);
  label.copy(0, 0, WinGui, 2, 2);
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
void gui::textstring_place(const img &FontImg, const std::string &TextString,
                   img& Dst, u_long x, u_long y)
{
  #ifndef NDEBUG
  if(x > Dst.w_summ - utf8_size(TextString) * FontImg.w_cell)
    ERR ("gui::add_text - X overflow");
  if(y > Dst.h_summ - FontImg.h_cell)
    ERR ("gui::add_text - Y overflow");
  #endif

  u_int row = 0;                        // номер строки в текстуре шрифта
  u_int n = 0;                          // номер буквы в выводимой строке
  size_t text_size = TextString.size(); // число байт в строке

  for(size_t i = 0; i < text_size; ++i)
  {
    auto t = char_type(TextString[i]);
    if(t == SINGLE)
    {
      size_t col = FontMap1.find(TextString[i]);
      if(col == std::string::npos) col = 0;
      FontImg.copy(col, row, Dst, x + (n++) * FontImg.w_cell, y);
    }
    else if(t == UTF8_FIRST)
    {
      size_t col = FontMap2.find(TextString.substr(i,2));
      if(col == std::string::npos) col = 0;
      else col = FontMap1_len + col/2;
      FontImg.copy(col, row, Dst, x + (n++) * FontImg.w_cell, y);
    }
  }
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
  u_int row_width = WinGui.w_summ - Font.w_cell * 2;
  u_int row_height = Font.h_cell * 2;
  img RowInput{ row_width, row_height, color };

  // добавить текст, введенный пользователем
  u_int y = (row_height - Font.h_cell)/2;
  textstring_place(Font, StringBuffer, RowInput, Font.w_cell, y);
  cursor_text_row(Font, RowInput, utf8_size(StringBuffer));

  // скопировать на экран изображение поля ввода с добавленым текстом
  auto x = (WinGui.w_summ - RowInput.w_summ) / 2;
  y = WinGui.h_summ / 2 - 2 * WinParams.btn_h;
  RowInput.copy(0, 0, WinGui, x, y);
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
void gui::row_text(size_t id, u_int x, u_int y, u_int w, u_int h, const std::string &text)
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
  if(WinParams.xpos >= x && WinParams.xpos <= x+w && WinParams.ypos >= y && WinParams.ypos <= y+h)
  {
    element_over = ROW_MAP_NAME; // для обработки в "menu_selector"
    bg_color = over;

    if(mouse_press_left)
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
  Row.copy(0, 0, WinGui, x, y);
}


///
/// \brief gui::draw_list_select
/// \details Отображение списка выбора
///
void gui::select_list(u_int lx, u_int ly, u_int lw, u_int lh)
{
  img ListImg {lw, lh, {0xDD, 0xDD, 0xDD, 0xFF}};             // изображение списка
  ListImg.copy(0, 0, WinGui, lx, ly);

  u_int rh = Font18n.h_cell * 1.5f;     // высота строки
  u_int rw = lw - 4;                    // ширина строки
  u_int max_rows = (lh - 4) / (rh + 2); // число строк, которое может поместиться в списке

  u_int id = 0;
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
  switch (GuiMode)
  {
    case GUI_3D_MODE:
      cfg::map_view_save();
      GuiMode = GUI_MENU_LSELECT;
      WinParams.Sight[2] = 0.0f;                  // Убрать прицел
      GlContext->cursor_restore();                 // Включить указатель мыши
      GlContext->set_cursor_observer(WinParams);   // переключить обработчик смещения курсора
      GlContext->set_button_observer(WinParams);   // обработчик кнопок мыши
      GlContext->set_keyboard_observer(WinParams); // и клавиатуры
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
      WinParams.is_open = false;
      break;
  }
}


///
/// \brief Перенос данных в указанную область графической памяти
///
/// \details Прямоугольный фрагмент передается напрямую в память GPU поверх
/// текстуры HUD используя метод glTexSubImage2D. За счет этого весь HUD не
/// перерисовывается целиком каждый кадр заново, а только те его фрагменты
/// которые именяются, что дает более высокий FPS.
///
/// \param Image
/// \param x
/// \param y
///
void gui::sub_img(const img &Image, GLint x, GLint y)
{
#ifndef NDEBUG
  if(x > static_cast<int>(WinGui.w_summ) - static_cast<int>(Image.w_summ))
    ERR ("giu::sub_img - overload X");
  if(y > static_cast<int>(WinGui.h_summ) - static_cast<int>(Image.h_summ))
    ERR ("giu::sub_img - overload Y");
#endif

  glTexSubImage2D(GL_TEXTURE_2D, 0, x, y,            // top, left
                static_cast<GLsizei>(Image.w_summ),  // width
                static_cast<GLsizei>(Image.h_summ),  // height
                GL_RGBA, GL_UNSIGNED_BYTE,           // mode
                Image.uchar());                      // data
}


///
/// \brief gui::update
///
void gui::hud_draw(void)
{
  glBindTexture(GL_TEXTURE_2D, gui_texture);

  // счетчик FPS
  px bg = { 0xF0, 0xF0, 0xF0, 0xA0 }; // фон заполнения
  u_int fps_length = 4;               // количество символов в надписи
  img Fps {fps_length * Font15n.w_cell + 4, Font15n.h_cell + 2, bg};
  char line[5];                   // the expected string plus 1 null terminator
  std::sprintf(line, "%.4i", Space->FPS);
  textstring_place(Font15n, line, Fps, 2, 1);
  sub_img(Fps, 2, static_cast<GLint>(WinParams.Layout.height - Fps.h_summ - 2));

  // Координаты в пространстве
  u_int c_length = 60;               // количество символов в надписи
  img Coord {c_length * Font15n.w_cell + 4, Font15n.h_cell + 2, bg};
  char ln[60];                    // the expected string plus 1 null terminator
  std::sprintf(ln, "X:%+3.1f, Y:%+03.1f, Z:%+03.1f, a:%+04.3f, t:%+04.3f",
                  Eye.ViewFrom.x, Eye.ViewFrom.y, Eye.ViewFrom.z, Eye.look_a, Eye.look_t);
  textstring_place(Font15n, ln, Coord, 2, 1);
  sub_img(Coord, 2, 2);
}


///
/// \brief загрузка HUD в GPU
///
/// \details Пока HUD имеет упрощенный вид в форме полупрозрачной прямоугольной
/// области в нижней части окна. Эта область формируется в ранее очищеной HUD
/// текстуре окна и зазгружается в память GPU. Загрузка производится разово
/// в момент открытия штроки (cover_) за счет обработки флага "renew". Далее,
/// в процессе взаимодействия с окруженим трехмерной сцены, текструра хранится
/// в памяти. Небольшие локальные фрагменты (вроде FPS-счетчика) обновляются
/// напрямую через память GPU.
///
void gui::hud_load(void)
{
  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA,
           static_cast<GLint>(WinParams.Layout.width), static_cast<GLint>(WinParams.Layout.height),
           0, GL_RGBA, GL_UNSIGNED_BYTE, WinGui.uchar());

  // Панель инструментов для HUD в нижней части окна
  u_int h = 48;                            // высота панели инструментов HUD
  if(h > WinGui.h_summ) h = WinGui.h_summ; // не может быть выше GuiImg
  img HudPanel {WinGui.w_summ, h, bg_hud};
  sub_img(HudPanel, 0,
          static_cast<GLint>(WinGui.h_summ) -
          static_cast<GLint>(HudPanel.h_summ));
}


///
/// \brief Затенить текстуру GIU
///
void gui::obscure_screen(void)
{
  size_t i = 0;
  size_t max = WinGui.Data.size();
  while(i < max)
  {
    WinGui.Data[i++] = bg;
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
      cfg::map_view_load(Maps[row_selected - 1].Folder);
      GuiMode = GUI_3D_MODE;

      WinGui.clear();            // очистка элементов GUI окна
      hud_load();
      WinParams.Sight[2] = 4.0f; // активировать прицел
      Space->enable();
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
  u_int np = 0;       // счетчик значений
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
      S = static_cast<u_char>(nr * step);
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
void gui::button(ELEMENT_ID btn_id, u_long x, u_long y,
                     const std::string &Name, bool button_is_active)
{
  img Btn { WinParams.btn_w, WinParams.btn_h };

  if(button_is_active)
  {
    // Если указатель находится над кнопкой
    if( WinParams.xpos >= x && WinParams.xpos <= x + WinParams.btn_w &&
        WinParams.ypos >= y && WinParams.ypos <= y + WinParams.btn_h)
    {
      element_over = btn_id;

      // и если кнопку "кликнули" указателем мыши
      if(mouse_press_left)
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
    textstring_place(Font18s, Name, Btn, WinParams.btn_w/2 - t_width/2,
           WinParams.btn_h/2 - t_height/2);
  }
  else
  {
    textstring_place(Font18l, Name, Btn, WinParams.btn_w/2 - t_width/2,
           WinParams.btn_h/2 - t_height/2);
  }
  Btn.copy(0, 0, WinGui, x, y);
}


///
/// \brief Начальный экран приложения
///
void gui::menu_start(void)
{
  obscure_screen();
  title("Trick Rig");

  u_int x = WinParams.Layout.width/2 - WinParams.btn_w/2;   // X координата кнопки
  u_int y = WinParams.Layout.height/2 - WinParams.btn_h/2;  // Y координата кнопки
  button(BTN_CONFIG, x, y, u8"Настроить");

  y -= 1.5 * WinParams.btn_h;
  button(BTN_LOCATION, x, y, u8"Играть");

  y += 3 * WinParams.btn_h;
  button(BTN_CANCEL, x, y, u8"Закрыть");
}


///
/// \brief Окно выбора района
///
void gui::menu_map_select(void)
{
  obscure_screen();
  title("ВЫБОР КАРТЫ");

  // Список фиксированой ширины и один ряд кнопок размещается в центре окна на
  // расстоянии 1/8 высоты окна сверху и снизу, и 1/8 ширины окна по бокам.
  // Расстояние между списком и кнопками равно половине высоты кнопки.

  u_int y = WinParams.Layout.height/8; // отступ сверху (и снизу)
  u_int list_h = WinParams.Layout.height - y * 2 - WinParams.btn_h * 1.5f;
  u_int list_w = WinParams.minwidth - 4;
  u_int x = (WinParams.Layout.width - list_w)/2;  // отступ слева (и справа)

  select_list(x, y, list_w, list_h);

  x = WinGui.w_summ / 2 + 8;
  y = y + list_h + WinParams.btn_h/2;

  button(BTN_CANCEL, x, y, "Отмена");
  button(BTN_MAP_DELETE, x + WinParams.btn_w + 16, y, "Удалить", row_selected > 0);
  button(BTN_OPEN, x - (WinParams.btn_w + 16), y, "Старт", row_selected > 0);
  button(BTN_CREATE, x - (WinParams.btn_w + 16)*2, y, "Создать");

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
  obscure_screen();
  title("ВВЕДИТЕ НАЗВАНИЕ");

  if((WinParams.key == KEY_BACKSPACE) && // Удаление введенных символов
    ((WinParams.action == PRESS)||(WinParams.action == REPEAT)))
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
  auto x = WinGui.w_summ / 2 - static_cast<u_long>(WinParams.btn_w * 1.25);
  auto y = WinGui.h_summ / 2;
  button(BTN_ENTER_NAME, x, y, "OK", StringBuffer.length() > 0);

  x += WinParams.btn_w * 1.5;  // X координата кнопки
  button(BTN_LOCATION, x, y, "Отмена");
}


///
/// \brief gui::menu_config
///
void gui::menu_config(void)
{
  obscure_screen();
  title("НАСТРОЙКИ");

  int x = WinGui.w_summ / 2 - static_cast<u_long>(WinParams.btn_w/2);
  int y = WinGui.h_summ / 2;
  button(BTN_CANCEL, x, y, "Отмена");
}


///
/// \brief gui::draw_gui_menu
///
void gui::menu_draw(void)
{
  glBindTexture(GL_TEXTURE_2D, gui_texture);

  mouse_press_left = (WinParams.mouse == MOUSE_BUTTON_LEFT) && (WinParams.action == PRESS);

  switch (GuiMode)
  {
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
  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA,
               static_cast<GLint>(WinParams.Layout.width),
               static_cast<GLint>(WinParams.Layout.height),
               0, GL_RGBA, GL_UNSIGNED_BYTE, WinGui.uchar());

  if((WinParams.mouse == MOUSE_BUTTON_LEFT) &&
     (WinParams.action == RELEASE) &&
     (element_over != NONE))
  {
    button_click(element_over);
    WinParams.mouse = -1;   // сбросить флаг кнопки
    WinParams.action = -1;  // сбросить флаг действия
  }

  if(element_over == NONE)
  {
    WinParams.mouse = -1;   // сбросить флаг кнопки
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
  GlContext->set_char_observer(*this);
  GlContext->set_error_observer(WinParams);    // отслеживание ошибок
  GlContext->set_cursor_observer(WinParams);   // курсор мыши в окне
  GlContext->set_button_observer(WinParams);   // кнопки мыши
  GlContext->set_keyboard_observer(WinParams); // клавиши клавиатуры
  GlContext->set_position_observer(WinParams); // положение окна
  GlContext->add_size_observer(WinParams);     // размер окна
  GlContext->set_close_observer(WinParams);    // закрытие окна

  while(WinParams.is_open)
  {
    if((WinParams.key == KEY_ESCAPE) && (WinParams.action == RELEASE))
    {
      cancel();
      WinParams.key    = -1;
      WinParams.action = -1;
    }

    if(GuiMode == GUI_3D_MODE)
    {
      if(!Space->render()) cancel();
      hud_draw();
    } else {
      menu_draw();
    }

    /// Рендер окна с текстурами фреймбуфера и GIU
    ///
    /// Кадр сцены рендерится в изображение на (2D) "холсте"
    /// фреймбуфера, после чего это изображение в виде текстуры накладывается на
    /// прямоугольник окна. Курсор и дополнительные (HUD) элементы окна
    /// изображаются как наложеные сверху дополнительные текстуры
    glBindVertexArray(vao_quad_id);
    glDisable(GL_DEPTH_TEST);
    screenShaderProgram.use();
    screenShaderProgram.set_uniform("Cursor", WinParams.Sight);
    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
    screenShaderProgram.unuse();

    // переключить буфер окна и получить данные ввода пользователя
    GlContext->swap_buffers();
  }

}


///
/// \brief gui::character_event
/// \param ch
///
void gui::character_event(u_int ch)
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

} //tr
