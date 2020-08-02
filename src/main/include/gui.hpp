#ifndef GUI_HPP
#define GUI_HPP

#include "main.hpp"
#include "vbo.hpp"
#include "framebuf.hpp"
#include "config.hpp"
#include "tools.hpp"

using sys_clock = std::chrono::system_clock;
using colors = std::array<tr::float_color, 4>;

namespace tr
{

typedef void(*func_ptr)(void);
typedef void(*func_with_param_ptr)(uint);

enum FONT_STYLE { FONT_NORMAL, FONT_BOLD, FONT_COUNT };
enum STATES { ST_NORMAL, ST_OVER, ST_PRESSED, ST_DISABLE };

static const uint menu_border = 20;   // расстояние от меню до края окна

static const uint symbol_width = 7;
static const uint symbol_height = 14;
static const uint symbol_kerning = 2; // расстояние между символами в надписи на кнопке

static const uint title_height = 80; // Высота поля заголовка
static const uint row_height = 20;

static const uint btn_width = 200;
static const uint btn_height = 25;
static const uint btn_padding = 14;   // расстояние между кнопками

static const uint MIN_GUI_WIDTH = btn_width * 4.2; // минимально допустимая ширина окна
static const uint MIN_GUI_HEIGHT = btn_height * 4 + 8;  // минимально допустимая высота окна

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
/// \brief The gui class
///
class gui: public interface_gl_context
{
  public:
    gui(std::shared_ptr<trgl>& OpenGLContext, std::shared_ptr<glm::vec3> CameraLocation);
    ~gui(void) = default;

    static bool open;
    //virtual void resize_event(int width, int height);
    virtual void cursor_event(double x, double y);                             // x, y
    virtual void mouse_event(int _button, int _action, int _mods);                // _button, _action, _mods
    virtual void keyboard_event(int _key, int _scancode, int _action, int _mods); // _key, _scancode, _action, _mods
    //virtual void character_event(unsigned int) {}

    void render(void);
    void hud_enable(void);

  private:
    static int window_width;
    static int window_height;
    static unsigned int indices;
    bool hud_is_enabled = false;
    int FPS = 500;                     // частота кадров
    GLsizei fps_uv_data = 0;           // смещение данных FPS в буфере UV

    struct element_data {
        double x0 = 0; // left
        double y0 = 0; // top
        double x1 = 0; // left + width
        double y1 = 0; // top + heigth
        STATES state = ST_NORMAL; // Состояние
        GLsizeiptr xy_stride = 0;     // Адрес в VBO блока координат вешин
        GLsizeiptr rgba_stride = 0;   // Адрес в VBO данных цвета
        size_t label_size = 0;        // число символов в надписи
        func_ptr caller = nullptr;    // Адрес функции, вызываемой по нажатию
    };
    static std::vector<element_data> Buttons; // Блок данных кнопок
    static std::vector<element_data> Rows;    // Список строк

    GLuint vao_gui = 0;
    std::shared_ptr<trgl>& OGLContext;       // OpenGL контекст окна приложения
    std::shared_ptr<glm::vec3> ViewFrom;     // 3D координаты камеры вида

    void init_vao(void);

    static void clear(void);
    static void start_screen(void);
    static void config_screen(void);
    static void select_map(void);
    static std::vector<float> rect_xy(int left, int top, uint width, uint height);
    static std::vector<float> rect_rgba(float_color rgba);
    static std::vector<float> rect_uv(const std::string& Symbol);
    static void rectangle(uint left, uint top, uint width, uint height, float_color rgba);
    static element_data create_element(layout L, const std::string &Label,
                                       const colors& BgColor, const colors& HemColor,
                                       func_ptr new_caller, STATES state);
    static void title(const std::string& Label);
    static void button_append(const std::string& Label, func_ptr new_caller);
    static void textrow(uint left, uint top, const std::vector<std::string>& Text, uint sybol_width, uint height, uint kerning);
    static void button_move(element_data& Button, int x, int y);
    static std::pair<uint, uint> button_allocation(void);
    static void list_insert(const std::string& String, STATES state);
    static void close(void) { open = false; }

    void calc_fps(void);
    void hud_update(void);
};

}

#endif // GUI_HPP
