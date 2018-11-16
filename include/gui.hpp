#ifndef GUI_HPP
#define GUI_HPP

#include "main.hpp"
#include "config.hpp"

typedef std::vector<unsigned char> TRvuch;

namespace tr {

struct pixel
{
  unsigned char r = 0x00;
  unsigned char g = 0x00;
  unsigned char b = 0x00;
  unsigned char a = 0x00;
};

// состояние кнопки
enum BUTTON_STATE {
  ST_NORMAL,
  ST_OVER,
  ST_PRESSED
};

class gui
{
  private:
    pixel bg     {0xE0, 0xE0, 0xE0, 0xC0}; // фон заполнения неактивного окна
    pixel bg_hud {0x00, 0x88, 0x00, 0x40}; // фон панелей HUD (активного окна)
    TRvuch vecGUI {};                     // RGBA массив изображения GUI
    const std::wstring Font { L"_`”~!?@#$%^&*-+=(){}[]<>\\|/,.:;abcdefghijklmn\
opqrstuvwxyzABCDEFGHIJKLMNOPQRSTUYWXYZ0123456789 абвгдеёжзийклмнопрстуфхцчшщъы\
ьэюяАБВГДЕЁЖЗИЙКЛМНОПРСТУФХЦЧШЩЪЫЬЭЮЯ" };

    void add_hud_panel(UINT h=48, UINT w=UINT_MAX, UINT t=UINT_MAX, UINT l=0);
    void obscure(void);
    void add_button(BUTTON_ID, UINT x, UINT y, const std::wstring&);
    void button_body(TRvuch& Data, UINT w, UINT h, BUTTON_STATE);
    void add_text(const tr::image& Font, const std::wstring& text,
                  tr::image& Data, UINT x, UINT y);

  public:
    gui(void);
    UCHAR* data = nullptr;            // адрес RGBA массива GIU
    void make(void);                  // формирование изображения GIU окна
    void update(void);                // обновление кадра
};

} //tr
#endif // GUI_HPP
