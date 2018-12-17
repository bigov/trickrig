#include "gui.hpp"

#include <chrono>
#include <thread>

namespace tr {

gui::gui(void)
{
  FontMap1_len = FontMap1.length();
  TimeStart = std::chrono::system_clock::now();

  // Составить список карт в каталоге пользователя
  auto MapsDirs = dirs_list(cfg::user_dir()); // список директорий с картами
  for(auto &P: MapsDirs) { Maps.push_back(map(P, cfg::map_name(P))); }
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
void gui::add_text(const img &FontImg, const std::string &TextString,
                   img& Dst, u_long x, u_long y)
{
  #ifndef NDEBUG
  if(x > Dst.w_summ - utf8_size(TextString) * FontImg.w_cell)
    ERR ("gui::add_text - X overflow");
  if(y > Dst.h_summ - FontImg.h_cell)
    ERR ("gui::add_text - Y overflow");
  #endif

  u_int row = 0;                        // номер строки в текстуре шрифта
  size_t col = 0;                       // номер колонки в текстуре шрифта
  u_int n = 0;                          // номер буквы в выводимой строке
  size_t text_size = TextString.size(); // число байт в строке

  for(size_t i = 0; i < text_size; ++i)
  {
    auto t = char_type(TextString[i]);
    if(t == SINGLE)
    {
      col = FontMap1.find(TextString[i]);
      if(col == std::string::npos) col = 0;
      FontImg.copy(col, row, Dst, x + (n++) * FontImg.w_cell, y);
    }
    else if(t == UTF8_FIRST)
    {
      col = FontMap2.find(TextString.substr(i,2));
      if(col == std::string::npos) col = 0;
      else col = FontMap1_len + col/2;
      FontImg.copy(col, row, Dst, x + (n++) * FontImg.w_cell, y);
    }
  }
}


///
/// \brief Начальный экран приложения
///
void gui::menu_start(void)
{
  obscure_screen();
  screen_title("Trick Rig");

  u_int x = AppWin.width/2 - AppWin.btn_w/2;   // X координата кнопки
  u_int y = AppWin.height/2 - AppWin.btn_h/2;  // Y координата кнопки
  add_button(BTN_CONFIG, x, y, u8"Настроить");

  y -= 1.5 * AppWin.btn_h;
  add_button(BTN_LOCATION, x, y, u8"Играть");

  y += 3 * AppWin.btn_h;
  add_button(BTN_CANCEL, x, y, u8"Закрыть");
}


///
/// \brief Создание заголовка экрана
/// \param title
///
void gui::screen_title(const std::string &title)
{
  px color {0xFF, 0xFF, 0xDD, 0xFF};
  img label{ GuiImg.w_summ - 4, Font18s.h_cell * 2 - 4, color};

  auto x = GuiImg.w_summ/2 - utf8_size(title) * Font18s.w_cell / 2;
  add_text(Font18s, title, label, x, Font18s.h_cell/2);
  label.copy(0, 0, GuiImg, 2, 2);

  return;
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
  screen_title(u8"ВВЕДИТЕ НАЗВАНИЕ");

  // Ввод названия
  if((AppWin.key == KEY_BACKSPACE) && (AppWin.action == PRESS))
  {
    if (user_input.length() == 0) return;
    if(char_type(user_input[user_input.size()-1]) != SINGLE)
    { user_input.pop_back(); } // если это UTF-8, то удаляем два байта
    user_input.pop_back();
    AppWin.key = -1;
  }

  // строка ввода текста
  add_input_string(Font18n);

  // две кнопки
  auto x = GuiImg.w_summ / 2 - static_cast<u_long>(AppWin.btn_w * 1.25);
  auto y = GuiImg.h_summ / 2;
  add_button(BTN_ENTER_NAME, x, y, "OK", user_input.length() > 0);

  x += AppWin.btn_w * 1.5;  // X координата кнопки
  add_button(BTN_LOCATION, x, y, u8"Отмена");

  return;
}

///
/// \brief Нарисовать поле ввода текстовой строки
/// \param _Fn - шрифт
///
/// \details Рисует указанным шрифтом, в фиксированной позиции, на всю
///  ширину экрана
///
void gui::add_input_string(const img &Font)
{
  px color = {0xF0, 0xF0, 0xF0, 0xFF};
  u_int row_width = GuiImg.w_summ - Font.w_cell * 2;
  u_int row_height = Font.h_cell * 2;
  img RowInput{ row_width, row_height, color };

  // добавить текст, введенный пользователем
  u_int y = (row_height - Font.h_cell)/2;
  add_text(Font, user_input, RowInput, Font.w_cell, y);

  add_text_cursor(Font, RowInput, utf8_size(user_input));

  // скопировать на экран изображение поля ввода с добавленым текстом
  auto x = (GuiImg.w_summ - RowInput.w_summ) / 2;
  y = GuiImg.h_summ / 2 - 2 * AppWin.btn_h;
  RowInput.copy(0, 0, GuiImg, x, y);

  return;
}

///
/// \brief gui::add_text_cursor
/// \param _Fn       шрифт ввода
/// \param _Dst      строка ввода
/// \param position  номер позиции курсора в строке ввода
///
void gui::add_text_cursor(const img &_Fn, img &_Dst, size_t position)
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
/// \brief gui::cover_config
///
void gui::menu_config(void)
{
  obscure_screen();
  screen_title(u8"НАСТРОЙКИ");

  int x = GuiImg.w_summ / 2 - static_cast<u_long>(AppWin.btn_w/2);
  int y = GuiImg.h_summ / 2;
  add_button(BTN_CANCEL, x, y, u8"Отмена");
}


///
/// \brief Окно выбора района
///
void gui::menu_map_select(void)
{
  obscure_screen();
  screen_title(u8"ВЫБОР КАРТЫ");

  static bool pr = true;
  for(auto p: Maps)
  {
    //p.Name - назначенное имя карты
  }


  // Курсор выбора
  std::string title { user_input };
  px color = {0xF0, 0xF0, 0xF0, 0xFF};
  auto cursor_width = GuiImg.w_summ - Font18n.w_cell * 2;
  u_int cursor_height = Font18n.h_cell * 2;
  img Cursor{ cursor_width, cursor_height, color};

  // добавить текст
  auto x = (cursor_width - title.length() * Font18n.w_cell) / 2;
  u_long y = (cursor_height - Font18n.h_cell) / 2;
  add_text(Font18n, title, Cursor, x, y);

  // скопировать на экран
  x = (GuiImg.w_summ - Cursor.w_summ) / 2;
  y = GuiImg.h_summ / 2 - 2 * AppWin.btn_h;
  Cursor.copy(0, 0, GuiImg, x, y);

  x = GuiImg.w_summ / 2 - static_cast<u_long>(AppWin.btn_w/2);
  y = GuiImg.h_summ / 2;

  add_button(BTN_OPEN, x, y, u8"Старт", user_input.length() > 0);
  add_button(BTN_CREATE, x - AppWin.btn_w - 16, y, u8"Создать");
  add_button(BTN_CANCEL, x + AppWin.btn_w + 16, y, u8"Отмена");
}


///
/// \brief gui::new_map_create
///
void gui::new_map_create(void)
{
  // Каталог пользователя
  cfg::create_map(user_input);
}


///
/// \brief gui::button_click - Обработчик нажатия клавиш GUI интерфейса
///
void gui::button_click(BUTTON_ID id)
{
  AppWin.pInputBuffer = nullptr; // Во всех режимах, кроме GUI_MENU_CREATE,
                                 // строка ввода отключена

  if(AppWin.mode == GUI_HUD3D) return;

  switch(id)
  {
    case BTN_OPEN:
      AppWin.mode = GUI_HUD3D;
      AppWin.Cursor[2] = 4.0f;
      AppWin.set_mouse_ptr = -1;
      break;
    case BTN_CONFIG:
      AppWin.mode = GUI_MENU_CONFIG;
      break;
    case BTN_LOCATION:
      AppWin.mode = GUI_MENU_LSELECT;
      break;
    case BTN_CREATE:
      user_input.clear();
      AppWin.pInputBuffer = &user_input;       // Включить пользовательский ввод
      AppWin.mode = GUI_MENU_CREATE;
      break;
    case BTN_ENTER_NAME:
      AppWin.mode = GUI_MENU_LSELECT;
      new_map_create();
      break;
    case BTN_CANCEL:
      cancel();
      break;
    case NONE:
      break;
  }

  AppWin.mouse = -1;   // сбросить флаг кнопки
  AppWin.action = -1;  // сбросить флаг действия
}

///
/// \brief gui::cancel Отмена текущего режима
///
void gui::cancel(void)
{
  switch (AppWin.mode)
  {
    case GUI_HUD3D:
      AppWin.mode = GUI_MENU_LSELECT;
      AppWin.Cursor[2] = 0.0f;  // Убрать прицел
      AppWin.set_mouse_ptr = 1; // Включить указатель мыши
      break;
    case GUI_MENU_LSELECT:
      AppWin.mode = GUI_MENU_START;
      break;
    case GUI_MENU_CREATE:
      AppWin.mode = GUI_MENU_LSELECT;
      break;
    case GUI_MENU_CONFIG:
      AppWin.mode = GUI_MENU_START;
      break;
    case GUI_MENU_START:
      AppWin.run = false;
      break;
  }

  AppWin.key    = -1;
  AppWin.action = -1;
}

///
/// \brief gui::draw_gui_menu
///
void gui::draw_gui_menu(void)
{
  if((AppWin.mouse == MOUSE_BUTTON_LEFT) &&
     (AppWin.action == RELEASE)) { button_click(button_over); }

  // При каждом рисовании каждой кнопки проверяются координаты указателя мыши.
  // Если указатель находится над кнопкой, то кнопка изображается другим цветом
  // и ее ID присваивается переменной "button_over"
  button_over = NONE;

  switch (AppWin.mode)
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

  // обновляем картинку меню в виде текстуры каждый кадр
  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA,
               static_cast<GLint>(AppWin.width),
               static_cast<GLint>(AppWin.height),
               0, GL_RGBA, GL_UNSIGNED_BYTE, GuiImg.uchar());
  return;
}

///
/// Загрузить в текстурный буфер стартовую заставку
///
void gui::draw_headband(void)
{
  GLint level_of_details = 0; GLint frame = 0;

  glTexImage2D(GL_TEXTURE_2D, level_of_details, GL_RGBA,
               static_cast<GLsizei>(headband.w_summ),
               static_cast<GLsizei>(headband.h_summ),
               frame, GL_RGBA, GL_UNSIGNED_BYTE, headband.uchar());
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
void gui::draw(void)
{
  if(AppWin.resized) GuiImg.resize(AppWin.width, AppWin.height);

  if((AppWin.key == KEY_ESCAPE) && (AppWin.action == RELEASE))
    cancel();

  if(AppWin.mode == GUI_HUD3D)
    refresh();
  else
    draw_gui_menu();

  AppWin.resized = false;
  return;
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
  if(x > static_cast<int>(GuiImg.w_summ) - static_cast<int>(Image.w_summ))
    ERR ("giu::sub_img - overload X");
  if(y > static_cast<int>(GuiImg.h_summ) - static_cast<int>(Image.h_summ))
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
void gui::refresh(void)
{
  if(AppWin.resized) load_hud();

  // счетчик FPS
  px bg = { 0xF0, 0xF0, 0xF0, 0xA0 }; // фон заполнения
  u_int fps_length = 4;               // количество символов в надписи
  img Fps {fps_length * Font15n.w_cell + 4, Font15n.h_cell + 2, bg};
  char line[5];                   // the expected string plus 1 null terminator
  std::sprintf(line, "%.4i", AppWin.fps);
  add_text(Font15n, line, Fps, 2, 1);
  sub_img(Fps, 2, static_cast<GLint>(AppWin.height - Fps.h_summ - 2));

  // Координаты в пространстве
  u_int c_length = 60;               // количество символов в надписи
  img Coord {c_length * Font15n.w_cell + 4, Font15n.h_cell + 2, bg};
  char ln[60];                    // the expected string plus 1 null terminator
  std::sprintf(ln, "X:%+3.1f, Y:%+03.1f, Z:%+03.1f, a:%+04.3f, t:%+04.3f",
                  Eye.ViewFrom.x, Eye.ViewFrom.y, Eye.ViewFrom.z, Eye.look_a, Eye.look_t);
  add_text(Font15n, ln, Coord, 2, 1);
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
void gui::load_hud(void)
{
  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA,
           static_cast<GLint>(AppWin.width), static_cast<GLint>(AppWin.height),
           0, GL_RGBA, GL_UNSIGNED_BYTE, GuiImg.uchar());

  // Панель инструментов для HUD в нижней части окна
  u_int h = 48;                            // высота панели инструментов HUD
  if(h > GuiImg.h_summ) h = GuiImg.h_summ; // не может быть выше GuiImg
  img HudPanel {GuiImg.w_summ, h, bg_hud};
  sub_img(HudPanel, 0,
          static_cast<GLint>(GuiImg.h_summ) -
          static_cast<GLint>(HudPanel.h_summ));
}

///
/// \brief Затенить текстуру GIU
///
void gui::obscure_screen(void)
{
  size_t i = 0;
  size_t max = GuiImg.Data.size();
  while(i < max)
  {
    GuiImg.Data[i++] = bg;
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
void gui::button_body(img &D, BUTTON_STATE s)
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
void gui::add_button(BUTTON_ID btn_id, u_long x, u_long y,
                     const std::string &Name, bool button_is_active)
{
  img Btn { AppWin.btn_w, AppWin.btn_h };

  if(button_is_active)
  {
    // Если указатель находится над кнопкой
    if( AppWin.xpos >= x && AppWin.xpos <= x + AppWin.btn_w &&
        AppWin.ypos >= y && AppWin.ypos <= y + AppWin.btn_h)
    {
      button_over = btn_id;

      // и если кнопку "кликнули" указателем мыши
      if((AppWin.mouse == MOUSE_BUTTON_LEFT) &&
         (AppWin.action == PRESS))
      {
        button_body(Btn, ST_PRESSED);
      }
      else
      {
        button_body(Btn, ST_OVER);
      }
    }
    else
    {
      button_body(Btn, ST_NORMAL);
    }
  }
  else
  {
    button_body(Btn, ST_OFF);
  }

  auto t_width = Font18s.w_cell * utf8_size(Name);
  auto t_height = Font18s.h_cell;

  if(button_is_active)
  {
    add_text(Font18s, Name, Btn, AppWin.btn_w/2 - t_width/2,
           AppWin.btn_h/2 - t_height/2);
  }
  else
  {
    add_text(Font18l, Name, Btn, AppWin.btn_w/2 - t_width/2,
           AppWin.btn_h/2 - t_height/2);
  }
  Btn.copy(0, 0, GuiImg, x, y);
}

} //tr
