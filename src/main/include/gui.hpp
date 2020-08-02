#ifndef GUI_HPP
#define GUI_HPP

#include "main.hpp"
#include "vbo.hpp"
#include "framebuf.hpp"
#include "config.hpp"
#include "image.hpp"
#include "space.hpp"

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

extern std::unique_ptr<glsl> Program2d;            // построение 2D элементов

///
/// \brief The gui class
///
class gui: public interface_gl_context
{
  public:
    gui(std::shared_ptr<trgl>& OpenGLContext);
    ~gui(void) = default;

    static bool open;
    virtual void resize_event(int width, int height);
    virtual void cursor_event(double x, double y);                             // x, y
    virtual void mouse_event(int _button, int _action, int _mods);                // _button, _action, _mods
    virtual void keyboard_event(int _key, int _scancode, int _action, int _mods); // _key, _scancode, _action, _mods
    virtual void focus_lost_event();
    //virtual void character_event(unsigned int) {}

    void render(void);
    static void hud_enable(void);

  private:
    static int window_width;
    static int window_height;
    static unsigned int indices;
    static bool hud_is_enabled;
    int FPS = 500;                     // частота кадров
    static GLsizei fps_uv_data;           // смещение данных FPS в буфере UV

    static std::unique_ptr<space_3d> Space3d;     // = nullptr;
    static bool RUN_3D;
    static GLuint vao2d;
    static glm::vec3 Cursor3D;               // положение и размер прицела

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
    static std::shared_ptr<trgl> OGLContext;   // OpenGL контекст окна приложения
    std::shared_ptr<glm::vec3> ViewFrom; // 3D координаты камеры вида
    static std::unique_ptr<glsl> ShowFrameBuf; // Шейдерная программа GUI

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
    static void map_open(uint map_id);
    static void close_map(void);
    static void close(void) { open = false; }

    void calc_fps(void);
    void hud_update(void);

    static void mode_3d(void);
    static void mode_2d(void);

    void fbuf_program_init(void);
    void framebuf_show(void);

};

}

#endif // GUI_HPP
