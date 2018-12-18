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
    img WinGui { 0, 0 };                 // GUI/HUD текстура окна приложения
    px color_title {0xFF, 0xFF, 0xDD, 0xFF}; // фон заголовка

    struct map{
        map(const std::string &f, const std::string &n): Folder(f), Name(n){}
        std::string Folder;
        std::string Name;
    };

    std::vector<map> Maps {};             // список карт

    enum BUTTON_ID {                      // Идентификаторы кнопок GIU
      BTN_OPEN,
      BTN_CANCEL,
      BTN_CONFIG,
      BTN_LOCATION,
      BTN_CREATE,
      BTN_ENTER_NAME,
      NONE
    };

    BUTTON_ID button_over = NONE;      // Над какой GIU кнопкой курсор
    size_t row_ower       = 0;         // над какой строкой курсор мыши

    // "FontMap1" - однобайтовые символы
    const std::string FontMap1 { u8"_'\"~!?@#$%^&*-+=(){}[]<>\\|/,.:;abcdefghi"
                                 "jklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUYWXYZ0"
                                 "123456789 "};
    // "FontMap2" - каждый символ занимает по два байта
    const std::string FontMap2 { u8"абвгдеёжзийклмнопрстуфхцчшщъыьэюяАБВГДЕЁЖЗ"
                                 "ИЙКЛМНОПРСТУФХЦЧШЩЪЫЬЭЮЯ" };
    u_int FontMap1_len = 0; // значение будет присвоено в конструкторе класса

    u_int f_len = 160; // количество символов в текстуре шрифта
    img Font12n { "../assets/font_07x12_nr.png", f_len }; //шрифт 07х12 (норм)
    img Font15n { "../assets/font_08x15_nr.png", f_len }; //шрифт 08х15 (норм)
    img Font18n { "../assets/font_10x18_nr.png", f_len }; //шрифт 10x18 (норм)
    img Font18s { "../assets/font_10x18_sh.png", f_len }; //шрифт 10x18 (тень)
    img Font18l { "../assets/font_10x18_lt.png", f_len }; //шрифт 10x18 (светл)

    img headband {"../assets/quad.png"}; // Текстура заставки

    std::string user_input {};  // строка ввода пользователя

    void load_hud(void);
    void obscure_screen(void);
    void draw_button(BUTTON_ID id, u_long x, u_long y, const std::string& Name,
                bool button_is_active = true );
    void button_body(img &Data, BUTTON_STATE);
    void add_text(const img &FontImg, const std::string& TextString,
                  img& Data, u_long x, u_long y);
    void draw_text_cursor(const img &_Fn, img &_Dst, size_t position);
    void draw_title(const std::string& title);
    void draw_input(const img &_Fn);
    void draw_text_row(size_t id, u_int x, u_int y, u_int w, u_int h, const std::string &);
    void draw_list_select(const v_str &, u_int x, u_int y, u_int w, u_int h, size_t i = 0);
    void sub_img(const img &Image, GLint x, GLint y);
    void draw_gui_menu(void);
    void menu_map_create(void);
    void menu_map_select(void);
    void menu_start(void);
    void menu_config(void);
    void button_click(BUTTON_ID);
    void cancel(void);
    void new_map_create(void);
    void refresh(void);      // обновление кадра

    std::chrono::time_point<std::chrono::system_clock> TimeStart;

  public:
    gui(void);
    void draw(void);         // формирование изображения GIU окна
    void draw_headband(void);// заставка
};

} //tr
#endif // GUI_HPP
