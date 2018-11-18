#include "gui.hpp"

namespace tr {

gui::gui(void)
{
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
void gui::add_text(const img &Src, const std::wstring& TextString,
                   img& Dst, UINT x, UINT y)
{
  UINT row = 0; // номер строки в текстуре шрифта
  UINT i = 0;   // номер символа в выводимой строке
  for (const wchar_t &wch: TextString)
  {
    Src.copy(Font.find(wch), row, Dst, x + (i++) * Src.w_cell, y);
  }
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
/// элементы управления и настройки приложения.
///
/// Из всех необходимых элементов собирается общее графическое изображение в
/// виде пиксельного массива и передается для рендера OpenGL изображения в
/// качестве накладываемой текстуры фрейм-буфера.
///
void gui::make(void)
{
  GuiImg.resize(AppWin.width, AppWin.height);

  // По-умолчанию указываем, что активной кнопки нет, процедура построения
  // кнопки установит свой BUTTON_ID, если курсор находится над ней
  AppWin.OverButton = NONE;

  if(AppWin.mode == OPEN)
  {
    add_hud_panel();
  }
  else
  {
    obscure();

    UINT x = AppWin.width/2 - AppWin.btn_w/2;   // X координата положения кнопки
    UINT y = AppWin.height/2 - AppWin.btn_h/2;  // Y координата кнопки
    add_button(BTN_CONFIG, x, y, L"Настроить");

    y -= 1.5 * AppWin.btn_h;
    add_button(BTN_OPEN, x, y, L"Играть");

    y += 3 * AppWin.btn_h;
    add_button(BTN_CLOSE, x, y, L"Закрыть");
  }

  return;
}

///
/// \brief gui::update
///
void gui::update(void)
{
  // Прямоугольная область с текстом накладывается напрямую в память GPU
  // поверх загруженой ранее текстуры GUI, используя метод glTexSubImage2D
  if(AppWin.mode == OPEN)
  {
    UINT label_width = 4 * TexFn15.w_summ/160 + 4;
    img Label {label_width, 17};
    size_t i = 0;
    size_t max = Label.Data.size();
    while(i < max)
    {
      Label.Data[i++] = { 0xF0, 0xF0, 0xF0, 0xA0 } ;
    }

    wchar_t line[5]; // the expected string plus 1 null terminator
    std::swprintf(line, 5, L"%.4i", AppWin.fps);

    add_text(TexFn15, line, Label, 2, 1);


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

    glTexSubImage2D(GL_TEXTURE_2D, 0,                               // place
                    2,                                              // left
                    static_cast<GLint>(AppWin.height - Label.h_summ - 2), // top
                    static_cast<GLsizei>(Label.w_summ),                  // width
                    static_cast<GLsizei>(Label.h_summ),                  // height
                    GL_RGBA, GL_UNSIGNED_BYTE,                      // mode
                    Label.Data.data());                             // data
  }
  return;
}

///
/// \brief Формирование на текстуре прямоугольной панели
/// \param height
/// \param width
/// \param top
/// \param left
///
void gui::add_hud_panel(UINT height, UINT width, UINT top, UINT left)
{
  // Высота не должна быть больше высоты окна
  if(AppWin.height < height) height = AppWin.height;

  // Если ширина не была указана, или указана больше ширины окна,
  // то установить ширину панели равной ширине окна
  if(UINT_MAX == width || width > AppWin.width) width = AppWin.width;

  // По-умолчанию расположить панель внизу
  if(UINT_MAX == top) top = AppWin.height - height;

  // Индекс первого элемента первого пикселя панели на текстуре GIU
  UINT i = static_cast<unsigned>(top * AppWin.width + left);

  // Индекс последнего элемента панели
  UINT i_max = i + static_cast<unsigned>(width * height);

  while(i < i_max) GuiImg.Data[i++] = bg_hud;

  return;
}

///
/// \brief Затенить текстуру GIU
///
void gui::obscure(void)
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
    case ST_NORMAL: default:
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
  UINT np = 0;       // счетчик значений
  double nr = 0.0;   // счетчик строк

  max += D.w_summ * (D.h_summ - 3);
  while (i < max)
  {
    D.Data[i++] = { static_cast<int>(line_bg.r) - S,
                    static_cast<int>(line_bg.g) - S,
                    static_cast<int>(line_bg.b) - S,
                    static_cast<int>(line_bg.a) };
    np++;
    if(np >= D.w_summ)
    {
      np = 0;
      nr += 1.0;
      S = static_cast<UCHAR>(nr * step);
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
void gui::add_button(BUTTON_ID btn_id, UINT x, UINT y, const std::wstring &D)
{
  img Btn { AppWin.btn_w, AppWin.btn_h };

  if( AppWin.xpos >= x && AppWin.xpos <= x + AppWin.btn_w &&
      AppWin.ypos >= y && AppWin.ypos <= y + AppWin.btn_h )
  {
    AppWin.OverButton = btn_id; // Указатель находится над кнопкой
    if(AppWin.mouse_lbutton_on) button_body(Btn, ST_PRESSED);
    else button_body(Btn, ST_OVER);
  }
  else
  {
    button_body(Btn, ST_NORMAL);
  }

  auto t_width = TexFn18.w_cell * D.length();
  auto t_height = TexFn18.h_cell;

  add_text(TexFn18, D, Btn, AppWin.btn_w/2 - t_width/2, AppWin.btn_h/2 - t_height/2);
  Btn.copy(0, 0, GuiImg, x, y);

  return;
}

///
/// \brief gui::uchar
/// \return
///
UCHAR* gui::uchar(void)
{
  return GuiImg.uchar();
}

} //tr
