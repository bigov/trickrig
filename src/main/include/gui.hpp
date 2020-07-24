#ifndef GUI_HPP
#define GUI_HPP

#include "wft/wft.hpp"

#include "main.hpp"
#include "vbo.hpp"
#include "framebuf.hpp"
#include "config.hpp"
#include "tools.hpp"

using sys_clock = std::chrono::system_clock;

namespace tr
{

typedef void(*func_ptr)(void);
typedef void(*func_with_param_ptr)(uint);

enum FONT_STYLE { FONT_NORMAL, FONT_BOLD, FONT_COUNT };
enum BTN_STATE { BTN_NORMAL, BTN_OVER, BTN_PRESSED, BTN_DISABLE, STATES_COUNT };

static const uint button_default_width = 140; // ширина кнопки GUI
static const uint button_default_height = 32; // высота кнопки GUI
static const uint label_default_height = 15;

static const uint MIN_GUI_WIDTH = button_default_width * 5.2; // минимально допустимая ширина окна
static const uint MIN_GUI_HEIGHT = button_default_height * 4 + 8;  // минимально допустимая высота окна

const uchar_color TextDefaultColor          { 0x24, 0x24, 0x24, 0xFF };
const uchar_color LinePressedDefaultBgColor { 0xFE, 0xFE, 0xFE, 0xFF };
const uchar_color LineOverDefaultBgColor    { 0xEE, 0xEE, 0xFF, 0xFF };
const uchar_color LineDisableDefaultBgColor { 0xCD, 0xCD, 0xCD, 0xFF };
const uchar_color LineNormalDefaultBgColor  { 0xE7, 0xE7, 0xE6, 0xFF };

class atlas;
class image;

extern atlas TextureFont;                    //текстура шрифта
extern std::unique_ptr<glsl> Program2d;      // построение 2D элементов
extern std::unique_ptr<frame_buffer> RenderBuffer; // рендер-буфер окна

extern void textstring_place(const atlas& FontImg, const std::string& TextString,
                   image& Dst, ulong x, ulong y);

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
class atlas: public image
{
  private:
    void resize(uint, uint, const uchar_color&) = delete;

  protected:
    uint columns = 1;      // число ячеек в строке
    uint rows =    1;      // число строк
    uint cell_width = 0;   // ширина ячейки в пикселях
    uint cell_height = 0;  // высота ячейки в пикселях

  public:
    atlas(const std::string &filename, uint new_cols = 1, uint new_rows = 1);

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
    std::string font_normal = cfg::AssetsDir + cfg::DS + "fonts" + cfg::DS + "FreeSans.ttf";
    std::string font_bold = cfg::AssetsDir + cfg::DS + "fonts" + cfg::DS + "FreeSansBold.ttf";
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
    button(const std::string& LabelText, func_ptr new_caller = nullptr,
           uint x = 0, uint y = 0, MODE new_mode = BUTTON,
           uint new_width = button_default_width, uint new_height = button_default_height);

    uint x = 0; uint y = 0;       // положение кнопки
    func_ptr caller = nullptr;    // функция, вызываемая по нажатию на кнопку

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
    static func_with_param_ptr callback_selected_row;
    static uint selected_row_id;

  public:
    menu_screen(void) = default;

    void init(uint new_width, uint new_height, const std::string& Title);
    void title_draw(const std::string& NewTitle);
    void button_add(uint x, uint y, const std::string& Label,
                    func_ptr new_caller = nullptr, BTN_STATE new_state = BTN_NORMAL);
    void list_add(const std::list<std::string>& ItemsList,
                  func_ptr fn_exit = nullptr, func_with_param_ptr fn_select = nullptr,
                  func_ptr fn_add = nullptr, func_ptr fn_delete = nullptr);
    bool cursor_event(double x, double y);
    func_ptr mouse_event(int mouse_button, int action, int mods);
    static void row_selected(void);
};


///
/// \brief The gui class
///
class gui: public interface_gl_context
{
  public:
    gui(std::shared_ptr<trgl>& OpenGLContext, std::shared_ptr<glm::vec3> CameraLocation);
    ~gui(void) = default;

    //virtual void resize_event(int width, int height);
    //virtual void cursor_event(double x, double y) {}                                //x, y
    virtual void mouse_event(int _button, int _action, int _mods);                // _button, _action, _mods
    virtual void keyboard_event(int _key, int _scancode, int _action, int _mods); // _key, _scancode, _action, _mods
    //virtual void character_event(unsigned int) {}

    uchar_color bg_color {0x00, 0x88, 0x00, 0x40}; // Фон панели HUD

    void render(void);
    void hud_enable(void);

  private:
    int window_width = 0;
    int window_height = 0;
    unsigned int indices = 0;
    bool hud_is_enabled = false;
    int FPS = 500;                                 // частота кадров
    GLsizei fps_uv_data = 0;                       // смещение данных FPS в буфере UV

    enum GUI_MODE { START_SCREEN, HUD };
    GUI_MODE mode = START_SCREEN;

    GLuint vao_gui = 0;
    std::shared_ptr<trgl>& OGLContext;   // OpenGL контекст окна приложения
    std::shared_ptr<glm::vec3> ViewFrom; // 3D координаты камеры вида
    vbo VBO_xy   { GL_ARRAY_BUFFER };    // координаты вершин
    vbo VBO_rgba { GL_ARRAY_BUFFER };    // цвет вершин
    vbo VBO_uv   { GL_ARRAY_BUFFER };    // текстурные координаты

    void init_vao(void);
    void vbo_clear(void);
    std::vector<float> rect_xy(int left, int top, uint width, uint height);
    std::vector<float> rect_rgba(float_color rgba);
    std::vector<float> rect_uv(const std::string& Symbol);
    void rectangle(uint left, uint top, uint width, uint height, float_color rgba);
    void textrow(uint left, uint top, const std::vector<std::string>& Text, uint sybol_width, uint height);

    void start_screen(void);
    void calc_fps(void);
    void hud_update(void);
};

}

#endif // GUI_HPP
