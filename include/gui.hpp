#ifndef GUI_HPP
#define GUI_HPP

#include "main.hpp"
#include "config.hpp"
#include "space.hpp"

namespace tr {

// состояние кнопки
enum BUTTON_STATE {
  ST_NORMAL,
  ST_OVER,
  ST_PRESSED,
  ST_OFF
};

enum GUI_MODES {   // режимы окна
  GUI_HUD3D,         // основной режим - без шторки
  GUI_MENU_START,    // начальное меню
  GUI_MENU_LSELECT,  // выбор игры
  GUI_MENU_CREATE,   // создание нового района
  GUI_MENU_CONFIG,   // настройки
};

class gui
{
  private:
    px bg      {0xE0, 0xE0, 0xE0, 0xC0};     // фон заполнения неактивного окна
    px bg_hud  {0x00, 0x88, 0x00, 0x40};     // фон панелей HUD (активного окна)
    img WinGui { 0, 0 };                     // GUI/HUD текстура окна приложения
    px color_title {0xFF, 0xFF, 0xDD, 0xFF}; // фон заголовка
    bool mouse_press_left = false;           // нажатие на левую кнопку мыши

    GLuint tex_hud_id   = 0;                 // id тектуры HUD

    struct map{
        map(const std::string &f, const std::string &n): Folder(f), Name(n){}
        std::string Folder;
        std::string Name;
    };

    std::vector<map> Maps {};            // список карт
    GUI_MODES GuiMode = GUI_MENU_START;  // режим окна приложения
    enum ELEMENT_ID {                    // Идентификаторы кнопок GIU
      BTN_OPEN,
      BTN_CANCEL,
      BTN_CONFIG,
      BTN_LOCATION,
      BTN_MAP_DELETE,
      BTN_CREATE,
      BTN_ENTER_NAME,
      ROW_MAP_NAME,
      NONE
    };

    ELEMENT_ID element_over = NONE;  // Над какой GIU кнопкой курсор
    size_t row_selected = 0;         // какая строка выбрана

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

    std::string user_input {};  // строка ввода пользователя
    space Space {};

    void hud_load(void);
    void obscure_screen(void);
    void button(ELEMENT_ID id, u_long x, u_long y, const std::string& Name,
                bool button_is_active = true );
    void button_make_body(img &Data, BUTTON_STATE);
    void textstring_place(const img &FontImg, const std::string& TextString,
                  img& Data, u_long x, u_long y);
    void cursor_text_row(const img &_Fn, img &_Dst, size_t position);
    void title(const std::string& title);
    void input_text_line(const img &_Fn);
    void row_text(size_t id, u_int x, u_int y, u_int w, u_int h, const std::string &);
    void select_list(const v_str &, u_int x, u_int y, u_int w, u_int h, size_t i = 0);
    void sub_img(const img &Image, GLint x, GLint y);
    void show_menu(evInput& ev);
    void menu_map_create(evInput& ev);
    void menu_map_select(void);
    void menu_start(void);
    void menu_config(void);
    void button_click(ELEMENT_ID);
    void cancel(void);
    void refresh_hud(void);      // обновление кадра
    void create_map(void);
    void remove_map(void);

    std::chrono::time_point<std::chrono::system_clock> TimeStart;

  public:
    gui(void);
    void draw(evInput &);      // формирование изображения GIU окна
};

} //tr
#endif // GUI_HPP
