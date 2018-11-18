#ifndef GUI_HPP
#define GUI_HPP

#include "main.hpp"
#include "config.hpp"

namespace tr {

// состояние кнопки
enum BUTTON_STATE {
  ST_NORMAL,
  ST_OVER,
  ST_PRESSED
};

class gui
{
  private:
    px bg      {0xE0, 0xE0, 0xE0, 0xC0}; // фон заполнения неактивного окна
    px bg_hud  {0x00, 0x88, 0x00, 0x40}; // фон панелей HUD (активного окна)
    img GuiImg { 0, 0 };                 // GUI-текстура окна приложения

    const std::wstring Font { L"_`”~!?@#$%^&*-+=(){}[]<>\\|/,.:;abcdefghijklmn\
opqrstuvwxyzABCDEFGHIJKLMNOPQRSTUYWXYZ0123456789 абвгдеёжзийклмнопрстуфхцчшщъы\
ьэюяАБВГДЕЁЖЗИЙКЛМНОПРСТУФХЦЧШЩЪЫЬЭЮЯ" };

    UINT f_len = 160; // количество символов в текстуре шрифта
    img TexFn15 { "../assets/font_08x15_nr.png", f_len }; //шрифт 08х15
    img TexFn18 { "../assets/font_10x18_sh.png", f_len }; //шрифт 10x18 с тенью

    void add_hud_panel(UINT h=48, UINT w=UINT_MAX, UINT t=UINT_MAX, UINT l=0);
    void obscure(void);
    void add_button(BUTTON_ID, UINT x, UINT y, const std::wstring&);
    void button_body(img &Data, BUTTON_STATE);
    void add_text(const img &Font, const std::wstring& TextString,
                  img& Data, UINT x, UINT y);

  public:
    gui(void);
    UCHAR* uchar(void);
    void make(void);                  // формирование изображения GIU окна
    void update(void);                // обновление кадра
};

} //tr
#endif // GUI_HPP
