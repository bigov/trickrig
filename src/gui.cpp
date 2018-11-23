#include "gui.hpp"

#include <chrono>
#include <thread>

namespace tr {

gui::gui(void)
{
  TimeStart = std::chrono::system_clock::now();
  return;
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
void gui::add_text(const img &FontImg, const std::wstring& TextString,
                   img& Dst, u_long x, u_long y)
{
  #ifndef NDEBUG
  if(x > Dst.w_summ - TextString.length() * FontImg.w_cell)
    ERR ("gui::add_text - X overflow");
  if(y > Dst.h_summ - FontImg.h_cell)
    ERR ("gui::add_text - Y overflow");
  #endif


  u_int row = 0; // номер строки в текстуре шрифта
  u_int i = 0;   // номер символа в выводимой строке
  for (const wchar_t &wch: TextString)
  {
    u_int id = 0;
    auto pos = FontMap.find(wch);
    if(pos != std::wstring::npos ) id = static_cast<u_int>(pos);
    FontImg.copy(id, row, Dst, x + (i++) * FontImg.w_cell, y);
  }
  return;
}

///
/// \brief Начальный экран приложения
///
void gui::cover_start(void)
{
  obscure_screen();
  screen_title(L"Trick Rig");

  u_int x = AppWin.width/2 - AppWin.btn_w/2;   // X координата кнопки
  u_int y = AppWin.height/2 - AppWin.btn_h/2;  // Y координата кнопки
  add_button(BTN_CONFIG, x, y, L"Настроить");

  y -= 1.5 * AppWin.btn_h;
  add_button(BTN_LOCATION, x, y, L"Играть");

  y += 3 * AppWin.btn_h;
  add_button(BTN_CLOSE, x, y, L"Закрыть");

  return;
}

///
/// \brief Создание заголовка экрана
/// \param title
///
void gui::screen_title(const std::wstring &title)
{
  px color {0xFF, 0xFF, 0xDD, 0xFF};
  img label{ GuiImg.w_summ - 4, Font18s.h_cell * 2 - 4, color};

  auto x = GuiImg.w_summ/2 - title.length() * Font18s.w_cell / 2;
  add_text(Font18s, title, label, x, Font18s.h_cell/2);
  label.copy(0, 0, GuiImg, 2, 2);

  return;
}

///
/// \brief Экран ввода названия для создания нового района
///
void gui::cover_create(void)
{
  obscure_screen();
  screen_title(L"ВВЕДИТЕ НАЗВАНИЕ");

  // Ввод названия
  if(AppWin.key_backspace)
  {
    if (AppWin.user_input.length() > 0) AppWin.user_input.pop_back();
    AppWin.key_backspace = false;
  }

  // строка ввода текста
  add_input_wstring(Font18n);

  // две кнопки
  auto x = GuiImg.w_summ / 2 - static_cast<u_long>(AppWin.btn_w * 1.25);
  auto y = GuiImg.h_summ / 2;
  add_button(BTN_ENTER_NAME, x, y, L"OK", AppWin.user_input.length() > 0);

  x += AppWin.btn_w * 1.5;  // X координата кнопки
  add_button(BTN_LOCATION, x, y, L"Отмена");

  return;
}

///
/// \brief Нарисовать поле ввода текстовой строки
/// \param _Fn - шрифт
///
/// Рисует указанным шрифтом, в фиксированной позиции, на всю ширину экрана
///
void gui::add_input_wstring(const img &_Fn)
{
  px color = {0xF0, 0xF0, 0xF0, 0xFF};
  u_int row_width = GuiImg.w_summ - _Fn.w_cell * 2;
  u_int row_height = _Fn.h_cell * 2;
  img RowInput{ row_width, row_height, color };

  // добавить текст, введенный пользователем
  u_int y = (row_height - _Fn.h_cell)/2;
  add_text(_Fn, AppWin.user_input, RowInput, _Fn.w_cell, y);

  add_text_cursor(_Fn, RowInput, AppWin.user_input.length());

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
  return;
}

///
/// \brief gui::cover_config
///
void gui::cover_config(void)
{
  obscure_screen();
  screen_title(L"НАСТРОЙКИ");

  return;
}

///
/// \brief Окно выбора района
///
void gui::cover_location(void)
{
  obscure_screen();
  screen_title(L"ВЫБОР РАЙОНА");

  // Курсор выбора
  std::wstring title { AppWin.user_input };
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

  x = GuiImg.w_summ / 2 - static_cast<u_long>(AppWin.btn_w * 1.25);
  y = GuiImg.h_summ / 2;

  add_button(BTN_OPEN, x, y, L"Старт", AppWin.user_input.length() > 0);

  x += AppWin.btn_w * 1.5;  // X координата кнопки
  add_button(BTN_CREATE, x, y, L"Создать");

  return;
}

///
/// \brief Создание элементов интерфейса окна
///
/// \details Окно приложения может иметь два состояния - открытое, в котором
/// происходит основной процесс, и режим настройка/управление (закрытое). В
/// обоих режимах поверх основного изображения OpenGL сцены накладываются
/// изображения дополнительных элементов. В первом случае это HUD и
/// контрольные элементы взаимодействия с контентом сцены, во втором -
/// это GIU элементы управления и настройки приложения.
///
/// Из всех необходимых элементов собирается общее графическое изображение в
/// виде текстурного массива и передается для рендера в OpenGL
///
void gui::draw(void)
{
  if(AppWin.newsize) GuiImg.resize(AppWin.width, AppWin.height);

  // По-умолчанию указываем, что активной кнопки нет, процедура построения
  // кнопки установит свой BUTTON_ID, если курсор находится над ней
  AppWin.OverButton = NONE;

  switch (AppWin.cover) {
    case COVER_OFF:
      refresh();
      break;
    case COVER_CONFIG:
      cover_config();
      break;
    case COVER_CREATE:
      cover_create();
      break;
    case COVER_LOCATION:
      cover_location();
      break;
    case COVER_START:
      cover_start();
      break;
   }

  // Если COVER включен, то его элементы строятся в составе GuiImg, в этом
  // случае обновляем всю область окна каждый кадр -
  if(AppWin.cover != COVER_OFF) glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA,
                                             static_cast<GLint>(AppWin.width),
                                             static_cast<GLint>(AppWin.height),
                                             0, GL_RGBA, GL_UNSIGNED_BYTE,
                                             GuiImg.uchar());

  AppWin.newsize = false;
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
  return;
}

///
/// \brief gui::update
///
void gui::refresh(void)
{
  if(AppWin.newsize) load_hud();

  px bg = { 0xF0, 0xF0, 0xF0, 0xA0 }; // фон заполнения
  u_int fps_length = 4;               // количество символов в надписи
  img Fps {fps_length * Font15n.w_cell + 4, Font15n.h_cell + 2, bg};
  wchar_t line[5]; // the expected string plus 1 null terminator
  std::swprintf(line, 5, L"%.4i", AppWin.fps);
  add_text(Font15n, line, Fps, 2, 1);
  sub_img(Fps, 2, static_cast<GLint>(AppWin.height - Fps.h_summ - 2));

/*
    TTF10.set_cursor(6, 14);
    TTF10.write_wstring(Label, { L"win:" + std::to_wstring(WinGl.width) +
                                 L"x" + std::to_wstring(WinGl.height) });

    std::swprintf(line, sz, L"X: %+06.1f", Eye.ViewFrom.x);
    TTF10.set_cursor(6, 26);
    TTF10.write_wstring(Label, line);

    std::swprintf(line, sz, L"Y: %+06.1f", Eye.ViewFrom.y);
    TTF10.set_cursor(6, 38);
    TTF10.write_wstring(Label, line);

    std::swprintf(line, sz, L"Z: %+06.1f", Eye.ViewFrom.z);
    TTF10.set_cursor(6, 50);
    TTF10.write_wstring(Label, line);
*/


  return;
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

  return;
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
  return;
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

  return;
}

///
/// \brief Формирование кнопки
///
/// Вначале формируется отдельное изображение кнопки, потом оно копируется
/// в указанное координатами (x,y) место окна.
///
///
void gui::add_button(BUTTON_ID btn_id, u_long x, u_long y, const std::wstring &Name,
                 bool button_is_active)
{
  img Btn { AppWin.btn_w, AppWin.btn_h };

  if(button_is_active)
  {
    // Если указатель находится над кнопкой
    if( AppWin.xpos >= x && AppWin.xpos <= x + AppWin.btn_w &&
        AppWin.ypos >= y && AppWin.ypos <= y + AppWin.btn_h)
    {
      AppWin.OverButton = btn_id;
      if(AppWin.mouse_lbutton_on)
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

  auto t_width = Font18s.w_cell * Name.length();
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

  return;
}

} //tr
