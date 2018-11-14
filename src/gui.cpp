#include "gui.hpp"

namespace tr {

// Текстура шрифта - файл размером 176x190 пиксей, 10 строк по 16 знаков,
// размер каждого знака 11х19 пикселей
tr::image ImgFont = get_png_img("../assets/176x190.png");
UINT ImgFont_rows = 10;
UINT ImgFont_cols = 16;
UINT s_w = 11; // ширина одного символа в пикселях
UINT s_h = 19; // высота символа в пикселях

gui::gui(void)
{
  TTFsmall.init(tr::cfg::get(TTF_FONT), 10);
  TTFsmall.set_color( 0x10, 0x10, 0x10, 0xFF );
  TTFsmall.load_chars(
    L"ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz:.,+- 0123456789" );

  TTFbig.init(tr::cfg::get(TTF_FONT), 12);
  TTFbig.set_color( 0x30, 0x30, 0x30, 0xFF );
  TTFbig.load_chars(
    L"ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz:.,+- 0123456789" );

  data = WinGui.data();
  return;
}

///
/// \brief Добавление текста
/// \param строка текста
/// \param массив пикселей, в который добавляется текст
/// \param х - координата
/// \param y - координата
///
//void gui::add_text(const std::wstring& text, tr::image& Data, UINT x, UINT y)
void gui::add_text(const std::wstring& Wt, tr::image& D, UINT x, UINT y)
{
  x -= s_w * Wt.length()/2; // Корректировка на длину строки
  y -= s_h/2;         // Корректировка на высоту символа

  UINT i_src = 0;                 // индекc на исходном изображении
  UINT i_dst = 4 * (x + y * D.w); // индекс целевого изображения
  UINT i_max = 0;

  UINT i = 0; // счетчик напечатанных символов

  for (const wchar_t& w_char: Wt)
  {
    auto w_char_id = Font.find(w_char); // индекс текстуры символа
    // размер текстуры символов 16х10. Находим строку и позицию в ней
    auto w_char_y = trunc(w_char_id/ImgFont_cols);
    auto w_char_x = w_char_id - w_char_y * ImgFont_cols;

    i_src = 4 * ( ImgFont.w * s_h * w_char_y + s_w * w_char_x );

    // интервал между соседними символами на текстурной карте = 1 пиксель
    // его надо убрать, иначе текст будет слишком разреженый
    UINT border = 1;

    i_dst = 4 * (i * s_w + x + y * D.w - border);

    for(UINT y = 0; y < s_h; y++)
    {
      i_max = i_dst + s_w * 4;
      while(i_dst < i_max)
      {
        if( ImgFont.Data[i_src+3] == 0xFF )
        {
          D.Data[i_dst++] = ImgFont.Data[i_src++] + 0x20; // сделаем символы
          D.Data[i_dst++] = ImgFont.Data[i_src++] + 0x20; // слегка светлее
          D.Data[i_dst++] = ImgFont.Data[i_src++] + 0x20; // увеличив яркость
        }
        i_dst++; i_src++;
      }
      i_dst += 4 * (D.w - s_w);
      i_src += 4 * (ImgFont.w - s_w);
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
  WinGui.clear();
  WinGui.resize(WinGl.width * WinGl.height * 4, 0x00);

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
    // координата X
    UINT x = WinGl.width/2 - btn_w/2;
    // координата Y
    UINT y = WinGl.height/2 - btn_h * 1.25;
    button(BTN_OPEN, x, y, L"start");

    y += 1.5 * btn_h;
    button(BTN_CLOSE, x, y, L"выход");

  }
  data = WinGui.data();
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
    tr::image Label {33, 16};
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
    TTFsmall.set_cursor(4, 0);
    TTFsmall.write_wstring(Label, line);

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
    WinGui[i++] = bg_hud.r;
    WinGui[i++] = bg_hud.g;
    WinGui[i++] = bg_hud.b;
    WinGui[i++] = bg_hud.a;
  }
  return;
}

///
/// \brief Затенить текстуру GIU
///
void gui::obscure(void)
{
  size_t i = 0;
  size_t max = WinGui.size();
  while(i < max)
  {
    WinGui[i++] = bg.r;
    WinGui[i++] = bg.g;
    WinGui[i++] = bg.b;
    WinGui[i++] = bg.a;
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
  image Btn {btn_w, btn_h};

  if( WinGl.xpos >= x && WinGl.xpos <= x + btn_w &&
      WinGl.ypos >= y && WinGl.ypos <= y + btn_h )
  {
    // Указатель находится над кнопкой
    WinGl.OverButton = btn_id;
    if(WinGl.mouse_lbutton_on) button_bg(Btn.Data, btn_w, btn_h, ST_PRESSED);
    else button_bg(Btn.Data, btn_w, btn_h, ST_OVER);
  }
  else
  {
    button_bg(Btn.Data, btn_w, btn_h, ST_NORMAL);
  }

  //UINT text_w = TTFbig.width(D);
  //UINT text_h = TTFbig.height();

  //TTFbig.set_cursor( (btn_w - text_w)/2, btn_h/2 - 0.7 * text_h );
  //TTFbig.write_wstring(Btn, D);
  add_text(D, Btn, btn_w/2, btn_h/2);

  UINT j = 0; // индекс элементов изображения кнопки

  for (UINT bt_y = 0; bt_y < btn_h; ++bt_y)
     for (UINT bt_x = 0; bt_x < btn_w * 4; ++bt_x)
  {
    size_t i = (y + bt_y) * WinGl.width * 4 + x * 4 + bt_x;
    WinGui[i] = Btn.Data[j++];
  }

  return;
}

} //tr
