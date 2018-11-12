#include "gui.hpp"

namespace tr {

gui::gui(void)
{
  TTF12.init(tr::cfg::get(TTF_FONT), 12);
  TTF12.set_color( 0x30, 0x30, 0x30, 0xFF );
  TTF12.load_chars(
    L"ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz: 0123456789" );

  return;
}

///
/// \brief Формирование на текстуре прямоугольной панели
/// \param Texture
/// \param height
/// \param width
/// \param top
/// \param left
///
void gui::make_panel(std::vector<UCHAR>& Texture,
     UINT height, UINT width, UINT top, UINT left)
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
    Texture[i++] = bg_hud.r;
    Texture[i++] = bg_hud.g;
    Texture[i++] = bg_hud.b;
    Texture[i++] = bg_hud.a;
  }
  return;
}

///
/// \brief Затенить текстуру окна
/// \param Texture
///
void gui::obscure(std::vector<UCHAR>& Texture)
{
  size_t i = 0;
  size_t max = Texture.size();
  while(i < max)
  {
    Texture[i++] = bg.r;
    Texture[i++] = bg.g;
    Texture[i++] = bg.b;
    Texture[i++] = bg.a;
  }
  return;
}

///
/// \brief Формирование кнопки
/// \param Texture Image
/// \param Font
/// \param Docket
///
void gui::make_button(TRvuch& Img, const std::wstring& D)
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
    Img[i] = Btn.Data[j++];
  }

  return;
}

} //tr
