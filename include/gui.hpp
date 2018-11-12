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
  public:
    gui(void);
    pixel bg     {0xE0, 0xE0, 0xE0, 0xC0}; // фон заполнения неактивного окна
    pixel bg_hud {0x00, 0x88, 0x00, 0x40}; // фон панелей HUD (активного окна)
    tr::ttf TTF12 {};                      // создание подписей на кнопках

    void make_panel(TRvuch& Texture,
                    UINT h=48, UINT w=UINT_MAX, UINT t=UINT_MAX, UINT l=0);
    void obscure(TRvuch&);
    void make_button(TRvuch&, const std::wstring&);
};

} //tr
#endif // GUI_HPP
