#include "gui.hpp"

namespace tr {

// Текстура шрифта 10х18 пикселей (с тенью)
tr::image TexFn18 = get_png_img("../assets/font_10x18_sh.png");

// Текстура шрифта 08х15 пикселей
tr::image TexFn15 = get_png_img("../assets/font_08x15_nr.png");

gui::gui(void)
{
  data = GuiRGBA.data();
  return;
}

///
/// \brief Добавление текста
/// \param текстура шрифта
/// \param строка текста
/// \param массив пикселей, в который добавляется текст
/// \param х - координата
/// \param y - координата
///
void gui::add_text(const image& Src, const std::wstring& Wt,
                   tr::image& Dst, UINT x, UINT y)
{
  UINT ch_w = Src.w / Font.length(); // ширина одного символа в пикселях
  UINT ch_h = Src.h;                 // высота символа в пикселях

  x -= ch_w * Wt.length()/2;    // Корректировка на длину строки
  y -= ch_h/2;                  // Корректировка на высоту символа

  UINT i_src = 0;                 // индекc на исходном изображении
  UINT i_dst = 4 * (x + y * Dst.w); // индекс целевого изображения
  UINT i_max = 0;

  UINT i = 0; // счетчик напечатанных символов

  for (const wchar_t& w_char: Wt)
  {
    auto w_char_id = Font.find(w_char);        // индекс символа в текстуре
    i_src = 4 * ( w_char_id * ch_w );        // индекс пикселя источника
    i_dst = 4 * (i * ch_w + x + y * Dst.w); // индекс пикселя приемника

    for(UINT y = 0; y < ch_h; y++)
    {
      i_max = i_dst + ch_w * 4;
      while(i_dst < i_max)
      {
        if( Src.Data[i_src+3] == 0xFF )
        {
          Dst.Data[i_dst++] = Src.Data[i_src++];
          Dst.Data[i_dst++] = Src.Data[i_src++];
          Dst.Data[i_dst++] = Src.Data[i_src++];
        }
        i_dst++; i_src++;
      }
      i_dst += 4 * (Dst.w - ch_w);
      i_src += 4 * (Src.w - ch_w);
    }
    i++;
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
  GuiRGBA.clear();
  GuiRGBA.resize(WinGl.width * WinGl.height * 4, 0x00);

  // По-умолчанию указываем, что активной кнопки нет, процедура построения
  // кнопки установит свой BUTTON_ID, если курсор находится над ней
  WinGl.OverButton = NONE;

  if(WinGl.is_open)
  {
    panel();
  }
  else
  {
    obscure();
    UINT x = WinGl.width/2 - WinGl.btn_w/2;       // X координата положения кнопки
    UINT y = WinGl.height/2 - WinGl.btn_h * 1.25; // Y координата кнопки
    button(BTN_OPEN, x, y, L"Start");

    y += 1.5 * WinGl.btn_h;
    button(BTN_CLOSE, x, y, L"Close");

  }
  data = GuiRGBA.data();
  return;
}

///
/// \brief gui::update
///
void gui::update(void)
{
  // Табличка с текстом на экране отображается в виде наложенного
  // на GL_TEXTURE2 изображения, которое шейдером складывается
  // с изображением трехмерной сцены, отрендереным во фреймбуфере.
  if(WinGl.is_open)
  {
    tr::image Label {36, 17};
    size_t i = 0;
    size_t max = Label.Data.size();
    while(i < max)
    {
      Label.Data[i++] = 0xF0;
      Label.Data[i++] = 0xF0;
      Label.Data[i++] = 0xF0;
      Label.Data[i++] = 0xA0;
    }

    wchar_t line[5]; // the expected string plus 1 null terminator
    std::swprintf(line, 5, L"%.4i", WinGl.fps);
    add_text(TexFn15, line, Label, Label.w/2, Label.h/2);

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
                    static_cast<GLint>(WinGl.height - Label.h - 2), // top
                    static_cast<GLsizei>(Label.w),                  // width
                    static_cast<GLsizei>(Label.h),                  // height
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
void gui::panel(UINT height, UINT width, UINT top, UINT left)
{
  // Высота не должна быть больше высоты окна
  if(WinGl.height < height) height = WinGl.height;

  // Если ширина не была указана, или указана больше ширины окна,
  // то установить ширину панели равной ширине окна
  if(UINT_MAX == width || width > WinGl.width) width = WinGl.width;

  // По-умолчанию расположить панель внизу
  if(UINT_MAX == top) top = WinGl.height - height;

  // Индекс первого элемента первого пикселя панели на текстуре GIU
  UINT i = static_cast<unsigned>(top * WinGl.width * 4 + left * 4);

  // Индекс последнего элемента панели
  UINT i_max = i + static_cast<unsigned>(width * height * 4);

  while(i < i_max)
  {
    GuiRGBA[i++] = bg_hud.r;
    GuiRGBA[i++] = bg_hud.g;
    GuiRGBA[i++] = bg_hud.b;
    GuiRGBA[i++] = bg_hud.a;
  }
  return;
}

///
/// \brief Затенить текстуру GIU
///
void gui::obscure(void)
{
  size_t i = 0;
  size_t max = GuiRGBA.size();
  while(i < max)
  {
    GuiRGBA[i++] = bg.r;
    GuiRGBA[i++] = bg.g;
    GuiRGBA[i++] = bg.b;
    GuiRGBA[i++] = bg.a;
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
void gui::button_bg(TRvuch& D, UINT w, UINT h, BUTTON_STATE s)
{
  double step = static_cast<double>(h-3)/256.0*24.0; // градации цвета

  pixel line_0  { 0xB6, 0xB6, 0xB3, 0xFF }; // верх и боковые
  pixel line_1 {};                    // блик (вторая линия)
  pixel line_bg {};                   // фоновый цвет
  pixel line_f  { 0x91, 0x91, 0x8C, 0xFF }; // нижняя

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

  UINT row_length = w * 4; // число значений в одной строке

  // верхняя линия
  size_t i = 0;
  size_t max = row_length;
  while(i < max)
  {
    D[i++] = line_0.r; D[i++] = line_0.g; D[i++] = line_0.b; D[i++] = line_0.a;
  }

  // вторая линия
  max += row_length;
  while(i < max)
  {
    D[i++] = line_1.r; D[i++] = line_1.g; D[i++] = line_1.b; D[i++] = line_1.a;
  }

  // основной фон
  UCHAR S = 0;         // коэффициент построчного уменьшения яркости
  UINT np = 0;         // счетчик значений
  double nr = 0.0;     // счетчик строк

  max += row_length * (h - 3);
  while (i < max)
  {
    D[i++] = S > line_bg.r ? 0 : line_bg.r - S;
    D[i++] = S > line_bg.g ? 0 : line_bg.g - S;
    D[i++] = S > line_bg.b ? 0 : line_bg.b - S;
    D[i++] = line_bg.a;

    np++;
    if(np >= row_length)
    {
      np = 0;
      nr += 1.0;
      S = static_cast<UCHAR>(nr * step);
    }

  }

  // нижняя линия
  max += row_length;
  while(i < max)
  {
    D[i++] = line_f.r; D[i++] = line_f.g; D[i++] = line_f.b; D[i++] = line_f.a;
  }

  // боковинки
  i = 0;
  while(i < max)
  {
    D[i++] = line_f.r; D[i++] = line_f.g; D[i++] = line_f.b; D[i++] = line_f.a;
    i += row_length - 8;
    D[i++] = line_f.r; D[i++] = line_f.g; D[i++] = line_f.b; D[i++] = line_f.a;
  }
  return;
}

///
/// \brief Формирование кнопки
/// \param Texture Image
/// \param Font
/// \param Docket
///
void gui::button(BUTTON_ID btn_id, UINT x, UINT y, const std::wstring& D)
{
  image Btn {WinGl.btn_w, WinGl.btn_h};

  if( WinGl.xpos >= x && WinGl.xpos <= x + WinGl.btn_w &&
      WinGl.ypos >= y && WinGl.ypos <= y + WinGl.btn_h )
  {
    // Указатель находится над кнопкой
    WinGl.OverButton = btn_id;
    if(WinGl.mouse_lbutton_on) button_bg(Btn.Data, WinGl.btn_w, WinGl.btn_h, ST_PRESSED);
    else button_bg(Btn.Data, WinGl.btn_w, WinGl.btn_h, ST_OVER);
  }
  else
  {
    button_bg(Btn.Data, WinGl.btn_w, WinGl.btn_h, ST_NORMAL);
  }

  add_text(TexFn18, D, Btn, WinGl.btn_w/2, WinGl.btn_h/2);
  UINT j = 0; // индекс данных писелей на изображении кнопки

  // Скопировать изображение кнопки на текстуру окна
  for (UINT bt_y = 0; bt_y < WinGl.btn_h; ++bt_y)
     for (UINT bt_x = 0; bt_x < WinGl.btn_w * 4; ++bt_x)
  {
    size_t i = (y + bt_y) * WinGl.width * 4 + x * 4 + bt_x;
    GuiRGBA[i] = Btn.Data[j++];
  }

  return;
}

} //tr
