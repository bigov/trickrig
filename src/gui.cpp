#include "gui.hpp"

namespace tr {

gui::gui(void)
{
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
     double height, double width, double top, double left)
{
  // Высота не должна быть больше высоты окна
  if(WinGl.height < height) height = WinGl.height;

  // Если ширина не была указана, или указана больше ширины окна,
  // то установить ширину панели равной ширине окна
  if(-1 == width || width > WinGl.width) width = WinGl.width;

  // По-умолчанию расположить панель внизу
  if(-1 == top) top = WinGl.height - height;

  // Индекс первого элемента первого пикселя панели на текстуре GIU
  size_t i = top * WinGl.width * 4 + left * 4;

  // Индекс последнего элемента панели
  size_t i_max = i + width * height * 4;

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
void gui::make_button(vuch& Img, tr::ttf& F, const std::wstring& D)
{
  UINT btn_w = 100;  // ширина кнопки
  UINT btn_h = 24;   // высота кнопки
  double x = WinGl.width/2 - btn_w/2; // координата X
  double y = WinGl.height/2 - btn_h/2; // координата Y
  image Btn {btn_w, btn_h};
  bool over = false;

  if( WinGl.xpos >= x &&
      WinGl.xpos <= x + btn_w &&
      WinGl.ypos >= y &&
      WinGl.ypos <= y + btn_h ) over = true;

  pixel body {};
  if(over)
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

  F.set_cursor(32, 2);
  F.write_wstring(Btn, D);

  int j = 0; // индекс элементов изображения кнопки

  for (UINT bt_y = 0; bt_y < btn_h; ++bt_y)
     for (UINT bt_x = 0; bt_x < btn_w * 4; ++bt_x)
  {
    size_t i = (y + bt_y) * WinGl.width * 4 + x * 4 + bt_x;
    Img[i] = Btn.Data[j++];
  }

  return;
}

object::object()
{
  return;
}

button::button(const ttf& f, const std::wstring& d): font(f), docket(d)
{
  width = padding_left + padding_right;
  height = padding_top + padding_bottom;
  return;
}

} //tr
