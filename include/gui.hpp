#ifndef GUI_HPP
#define GUI_HPP

#include "main.hpp"
#include "config.hpp"

namespace tr {

// состояние кнопки
enum BUTTON_STATE {
  ST_NORMAL,
  ST_OVER,
  ST_PRESSED,
  ST_OFF
};

class gui
{

  private:
    px bg      {0xE0, 0xE0, 0xE0, 0xC0}; // фон заполнения неактивного окна
    px bg_hud  {0x00, 0x88, 0x00, 0x40}; // фон панелей HUD (активного окна)
    img GuiImg { 0, 0 };                 // GUI/HUD текстура окна приложения

    const std::wstring FontMap { L"_`”~!?@#$%^&*-+=(){}[]<>\\|/,.:;abcdefghijk\
lmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUYWXYZ0123456789 абвгдеёжзийклмнопрстуфхцчш\
щъыьэюяАБВГДЕЁЖЗИЙКЛМНОПРСТУФХЦЧШЩЪЫЬЭЮЯ" };

    u_int f_len = 160; // количество символов в текстуре шрифта
    img Font12n { "../assets/font_07x12_nr.png", f_len }; //шрифт 07х12 (норм)
    img Font15n { "../assets/font_08x15_nr.png", f_len }; //шрифт 08х15 (норм)
    img Font18n { "../assets/font_10x18_nr.png", f_len }; //шрифт 10x18 (норм)
    img Font18s { "../assets/font_10x18_sh.png", f_len }; //шрифт 10x18 (тень)
    img Font18l { "../assets/font_10x18_lt.png", f_len }; //шрифт 10x18 (светл)

    void load_hud(void);
    void obscure_screen(void);
    void add_button(BUTTON_ID id, u_long x, u_long y, const std::wstring &Name,
                bool button_is_active = true );
    void button_body(img &Data, BUTTON_STATE);
    void add_text(const img &FontImg, const std::wstring& TextString,
                  img& Data, u_long x, u_long y);
    void add_text_cursor(const img &_Fn, img &_Dst, size_t position);
    void screen_title(const std::wstring &title);
    void add_input_wstring(const img &_Fn);
    void sub_img(const img &Image, GLint x, GLint y);
    void cover_create(void);
    void cover_location(void);
    void cover_start(void);
    void cover_config(void);
    std::chrono::time_point<std::chrono::system_clock> TimeStart;

  public:
    gui(void);
    void draw(void);                  // формирование изображения GIU окна
    void update(void);                // обновление кадра
};

} //tr
#endif // GUI_HPP
