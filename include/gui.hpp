#ifndef GUI_HPP
#define GUI_HPP

#include "main.hpp"
#include "ttf.hpp"

typedef std::vector<UCHAR> vuch;

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

    void make_panel(vuch& Texture,
                    double h=48, double w=-1, double t=-1, double l=0);
    void obscure(vuch&);
    void make_button(vuch&, tr::ttf&, const std::wstring&);
};

class object
{
  public:
    object();

  protected:
    double width = 0;
    double height = 0;
    double left = 0;
    double top = 0;
};

class button: public object
{
  public:
    button(const ttf&, const std::wstring&);

  protected:
    double padding_top = 2;
    double padding_right = 2;
    double padding_bottom = 2;
    double padding_left = 2;

    const ttf& font;
    const std::wstring& docket;
};

} //tr
#endif // GUI_HPP
