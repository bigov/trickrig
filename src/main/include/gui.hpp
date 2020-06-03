#ifndef GUI_HPP
#define GUI_HPP

#include "main.hpp"
#include "config.hpp"
#include "tools.hpp"

namespace tr
{
typedef void (*fn_pointer)(void);

enum FONT_STYLE { FONT_NORMAL, FONT_BOLD, FONT_COUNT };
enum BTN_STATE { BTN_NORMAL, BTN_OVER, BTN_PRESSED, BTN_DISABLE, STATES_COUNT };

const uint button_default_width = 120;
const uint button_default_height = 32;
const uint label_default_height = 24;

const uchar_color TextDefaultColor          { 0x24, 0x24, 0x24, 0xFF };
const uchar_color LinePressedDefaultBgColor { 0xFE, 0xFE, 0xFE, 0xFF };
const uchar_color LineOverDefaultBgColor    { 0xEE, 0xEE, 0xFF, 0xFF };
const uchar_color LineDisableDefaultBgColor { 0xCD, 0xCD, 0xCD, 0xFF };
const uchar_color LineNormalDefaultBgColor  { 0xE7, 0xE7, 0xE6, 0xFF };

class texture;
class image;

extern uint f_len; // количество символов в текстуре шрифта
extern texture Font12n; //шрифт 07х12 (норм)
extern texture Font15n; //шрифт 08х15 (норм)
extern texture Font18n; //шрифт 10x18 (норм)
extern texture Font18s; //шрифт 10x18 (тень)
extern texture Font18l; //шрифт 10x18 (светл)

extern void textstring_place(const texture& FontImg, const std::string& TextString,
                   image& Dst, ulong x, ulong y);

// Настройка пиксельных шрифтов
// ----------------------------
// "FontMap1" - однобайтовые символы
extern const std::string FontMap1;
// "FontMap2" - каждый символ занимает по два байта
extern const std::string FontMap2;
extern uint FontMap1_len; // значение будет присвоено в конструкторе класса

///
/// Класс для хранения в памяти избражений
///
class image
{
  protected:
    uint width = 0;    // ширина изображения в пикселях
    uint height = 0;   // высота изображения в пикселях
    void load(const std::string &filename);
    std::vector<uchar_color> Data {};

  public:
    // конструкторы
    image(void)                    = default;
    image(const image&)            = default;
    image& operator=(const image&) = default;
    image(const std::string& filename);
    image(uint new_width, uint new_height, const uchar_color& NewColor = { 0xFF, 0xFF, 0xFF, 0xFF });
    ~image(void)                   = default;

    void resize(uint new_width, uint new_height, const uchar_color& Color = {0x00, 0x00, 0x00, 0x00});
    auto get_width(void) const { return width; }
    auto get_height(void) const { return height; }
    void fill(const uchar_color& new_color);
    uchar* uchar_t(void) const;
    uchar_color* color_data(void) const;
    void put(image &dst, ulong x, ulong y) const;
    void paint_over(uint x, uint y, const image& Src);
};


// Служебный класс для хранения в памяти текстур
class texture: public image
{
  private:
    void resize(uint, uint, const uchar_color&) = delete;

  protected:
    uint columns = 1;      // число ячеек в строке
    uint rows =    1;      // число строк
    uint cell_width = 0;   // ширина ячейки в пикселях
    uint cell_height = 0;  // высота ячейки в пикселях

  public:
    texture(const std::string &filename, uint new_cols = 1, uint new_rows = 1);

    auto get_cell_width(void) const { return cell_width;  }
    auto get_cell_height(void) const { return cell_height; }
    void put(uint col, uint row, image &dst, ulong dst_x, ulong dst_y) const;
};


///
/// \brief The label class
///
class label: public image
{
  protected:
    std::string font_normal = cfg::AssetsDir + cfg::DS + "FreeSans.ttf";
    std::string font_bold = cfg::AssetsDir + cfg::DS + "FreeSansBold.ttf";
    std::string Text {};
    uchar_color TextColor = TextDefaultColor;
    int letter_space = 80; // % size of

  public:
    label(void) = default;
    label(const std::string& new_text, unsigned int new_height = 18,
          FONT_STYLE weight = FONT_NORMAL, uchar_color NewColor = TextDefaultColor);
};


///
/// \brief The button class
///
class button: public image
{
  public:
    enum MODE { BUTTON, LIST_ENTRY, MODES_COUNT };   // вид кнопки

    button(void)                     = default;
    button(const button&)            = default;
    button& operator=(const button&) = default;
    button(const std::string& LabelText, void(*new_caller)(void) = nullptr,
           uint x = 0, uint y = 0, MODE new_mode = BUTTON,
           uint new_width = button_default_width, uint new_height = button_default_height);

    uint x = 0; uint y = 0;             // положение кнопки
    fn_pointer caller = nullptr;        // функция, вызываемая по нажатию на кнопку

    bool state_update(BTN_STATE new_state);
    BTN_STATE state_get(void) const { return state;}

  protected:
    BTN_STATE state = STATES_COUNT;
    label Label {};
    MODE mode = MODES_COUNT;

    void draw_button(void);
    void draw_list_entry(void);
};


///
/// \brief The menu_screen class
///
class menu_screen: public image
{
  private:
    uchar_color ColorMainBg  { 0xE0, 0xE0, 0xE0, 0xC0 };
    uchar_color ColorTitleBg { 0xFF, 0xFF, 0xDD, 0xFF };
    uchar_color ColorTitleFg { 0x00, 0x00, 0x00, 0x00 };
    std::list<button> MenuItems {};
    uint title_height = 0;

  public:
    menu_screen(void) = default;

    void init(uint new_width, uint new_height, const std::string& Title);
    void title_draw(const std::string& NewTitle);
    void button_add(uint x, uint y, const std::string& Label, void(*new_caller)(void) = nullptr);
    uint list_add(const std::list<std::string>& ItemsList);
    bool cursor_event(double x, double y);
    fn_pointer mouse_event(int mouse_button, int action, int mods);
};

}

#endif // GUI_HPP
