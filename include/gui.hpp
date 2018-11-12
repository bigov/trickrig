#ifndef GUI_HPP
#define GUI_HPP

#include "main.hpp"
#include "config.hpp"
#include "ttf.hpp"

typedef std::vector<unsigned char> TRvuch;

namespace tr {

struct pixel
{
  unsigned char r = 0x00;
  unsigned char g = 0x00;
  unsigned char b = 0x00;
  unsigned char a = 0x00;
};

class gui
{
  private:
    pixel bg     {0xE0, 0xE0, 0xE0, 0xC0}; // фон заполнения неактивного окна
    pixel bg_hud {0x00, 0x88, 0x00, 0x40}; // фон панелей HUD (активного окна)
    ttf TTF12 {};                          // создание подписей на кнопках
    TRvuch WinGui {};                      // RGBA массив изображения GUI

    void panel(UINT h=48, UINT w=UINT_MAX, UINT t=UINT_MAX, UINT l=0);
    void obscure(void);
    void button(const std::wstring&);

  public:
    gui(void);
    UCHAR* data = nullptr;            // адрес RGBA массива GIU
    void make(void);                  // формирование изображения GIU окна
    void update(void);                // обновление кадра
};

} //tr
#endif // GUI_HPP
