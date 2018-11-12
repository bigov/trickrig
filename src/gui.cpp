#include "gui.hpp"

namespace tr {

gui::gui(void)
{
  TTF10.init(tr::cfg::get(TTF_FONT), 10);
  TTF10.set_color( 0x10, 0x10, 0x10, 0xFF );
  TTF10.load_chars(
    L"ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz:.,+- 0123456789" );

  TTF12.init(tr::cfg::get(TTF_FONT), 12);
  TTF12.set_color( 0x30, 0x30, 0x30, 0xFF );
  TTF12.load_chars(
    L"ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz:.,+- 0123456789" );

  data = WinGui.data();
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

  if(WinGl.is_open)
  {
    panel();
  }
  else
  {
    obscure();
    button(L"Open");
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
    TTF10.set_cursor(4, 0);
    TTF10.write_wstring(Label, line);

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

    glTexSubImage2D(GL_TEXTURE_2D, 0,
                    2,                           // координата X
                    WinGl.height - Label.h - 2,  // координата Y
                    static_cast<GLsizei>(Label.w),
                    static_cast<GLsizei>(Label.h),
                    GL_RGBA, GL_UNSIGNED_BYTE, Label.Data.data());
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
/// \brief Формирование кнопки
/// \param Texture Image
/// \param Font
/// \param Docket
///
void gui::button(const std::wstring& D)
{
  UINT btn_w = 100;  // ширина кнопки
  UINT btn_h = 24;   // высота кнопки
  UINT x = WinGl.width/2 - btn_w/2; // координата X
  UINT y = WinGl.height/2 - btn_h/2; // координата Y
  image Btn {btn_w, btn_h};

  pixel body {};
  if( WinGl.xpos >= x && WinGl.xpos <= x + btn_w &&
      WinGl.ypos >= y && WinGl.ypos <= y + btn_h )
  {
    body = {0xE0, 0xFF, 0xE0, 0xFF};
  }
  else
  {
    body = {0xE0, 0xE0, 0xE0, 0xFF};
  }

  size_t i = 0;
  size_t max = btn_w * 4;
  while(i < max)
  {
    Btn.Data[i++] = 0xB0;
    Btn.Data[i++] = 0xB0;
    Btn.Data[i++] = 0xB0;
    Btn.Data[i++] = 0xFF;
  }

  max = Btn.Data.size() - btn_w * 4;
  while(i < max)
  {
    Btn.Data[i++] = body.r;
    Btn.Data[i++] = body.g;
    Btn.Data[i++] = body.b;
    Btn.Data[i++] = body.a;
  }

  max = Btn.Data.size();
  while(i < max)
  {
    Btn.Data[i++] = 0xB0;
    Btn.Data[i++] = 0xB0;
    Btn.Data[i++] = 0xB0;
    Btn.Data[i++] = 0xFF;
  }

  i = 0;
  while(i < max)
  {
    Btn.Data[i+0] = 0xB0;
    Btn.Data[i+1] = 0xB0;
    Btn.Data[i+2] = 0xB0;
    Btn.Data[i+3] = 0xFF;
    i += btn_w * 4 - 4;
    Btn.Data[i+0] = 0xB0;
    Btn.Data[i+1] = 0xB0;
    Btn.Data[i+2] = 0xB0;
    Btn.Data[i+3] = 0xFF;
    i += 4;
  }

  TTF12.set_cursor(32, 2);
  TTF12.write_wstring(Btn, D);

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
