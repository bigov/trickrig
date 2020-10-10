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

typedef void(*func_ptr)(uint);
//typedef void(*func_with_param_ptr)(uint);

enum FONT_STYLE { FONT_NORMAL, FONT_BOLD, FONT_COUNT };
enum STATES { ST_NORMAL, ST_OVER, ST_PRESSED, ST_DISABLE };
enum ELEMENT_TYPES { GUI_BUTTON, GUI_ROWSLIST };

const uint menu_border_default = 20;   // расстояние от меню до края окна

const uint sym_width_default = 7;
const uint sym_height_default = 14;
const uint sym_kerning_default = 2; // расстояние между символами в надписи на кнопке

const uint title_height_default = 80; // Высота поля заголовка
const uint listrow_height_default = 20;

const uint btn_width_default = 200;
const uint btn_height_default = 25;
const uint btn_padding_default = 14;   // расстояние между кнопками

const uint MIN_GUI_WIDTH = btn_width_default * 4.2; // минимально допустимая ширина окна
const uint MIN_GUI_HEIGHT = btn_height_default * 4 + 8;  // минимально допустимая высота окна

const uchar_color TextColorDefault          { 0x24, 0x24, 0x24, 0xFF };
const uchar_color LinePressedBgColorDefault { 0xFE, 0xFE, 0xFE, 0xFF };
const uchar_color LineOverBgColorDefault    { 0xEE, 0xEE, 0xFF, 0xFF };
const uchar_color LineDisableBgColorDefault { 0xCD, 0xCD, 0xCD, 0xFF };
const uchar_color LineNormalBgColorDefault  { 0xE7, 0xE7, 0xE6, 0xFF };

const float_color DefaultBgColor {0.0f, 0.0f, 0.0f, 0.0f}; // цвет фона текста по-умолчанию

const float_color TitleBgColor   { 1.0f, 1.0f, 0.85f, 1.0f };
const float_color TitleHemColor  { 0.7f, 0.7f, 0.70f, 1.0f };

const colors BtnBgColor =
  { float_color { 0.89f, 0.89f, 0.89f, 1.0f }, // normal
    float_color { 0.95f, 0.95f, 0.95f, 1.0f }, // over
    float_color { 0.85f, 0.85f, 0.85f, 1.0f }, // pressed
    float_color { 0.85f, 0.85f, 0.85f, 0.1f }  // disabled
  };
const colors BtnHemColor=
  { float_color { 0.70f, 0.70f, 0.70f, 1.0f }, // normal
    float_color { 0.70f, 0.70f, 0.70f, 1.0f }, // over
    float_color { 0.70f, 0.70f, 0.70f, 1.0f }, // pressed
    float_color { 0.70f, 0.70f, 0.70f, 1.0f }  // disabled
  };
const colors ListBgColor =
  { float_color { 0.90f, 0.90f, 0.99f, 1.0f }, // normal
    float_color { 0.95f, 0.95f, 0.95f, 1.0f }, // over
    float_color { 1.00f, 1.00f, 1.00f, 1.0f }, // pressed
    float_color { 0.85f, 0.85f, 0.85f, 1.0f }  // disabled
  };
const colors ListHemColor=
  { float_color { 0.85f, 0.85f, 0.90f, 1.0f }, // normal
    float_color { 0.70f, 0.70f, 0.70f, 1.0f }, // over
    float_color { 0.90f, 0.90f, 0.90f, 1.0f }, // pressed
    float_color { 0.70f, 0.70f, 0.70f, 1.0f }  // disabled
  };

extern std::unique_ptr<glsl> Program2d;            // построение 2D элементов

struct confines {
  double x0 = 0; // left
  double y0 = 0; // top
  double x1 = 0; // right (left + width)
  double y1 = 0; // bottom (top + heigth)
};

struct glmem_ptr {
  GLsizeiptr offset = 0; // Адрес в VBO блока данных
  GLsizeiptr size = 0;   // Длина VBO блока данных
};

struct vbo_ptr {
  glmem_ptr xy;     // Адрес в VBO блока координат вершин
  glmem_ptr rgba;   // Адрес в VBO данных цвета
  glmem_ptr uv;     // Длина VBO блока координат текстуры
};

class face
{
public:
  face(const layout& Layout, const std::string& Symbol = " ", float_color BgColor = DefaultBgColor);
  ~face(void);

  void update_xy(const layout& L);
  void move_xy(const uint x, uint y);
  void update_rgba(const float_color& Color);
  void update_uv(const std::string& Symbol);

  bool uv_equal(const std::string& S) { return S == Char; }
  layout get_layout(void) const { return layout{Layout}; }

protected:
  face(void) = delete;
  face(const face&) = delete;
  face(face&&) = delete;
  face& operator=(const face&) = delete;
  face& operator=(face&&) = delete;

  void init(const layout& L, const std::string& Symbol, float_color BgColor);

  std::string Char {};
  vbo_ptr Addr {};
  layout Layout {};
};


///
/// \brief The input_ctrl class
///
class input_ctrl: public face
{
public:
  input_ctrl(const layout& L):face(L) {};

  void keyboard_event(int key, int scancode, int action, int mods);
  bool move_next(uint symbol_size);
  uint current_char(void);
  uint get_row_position(void)const { return row_position; };
  uint get_row_size(void)const { return row_size; }
  void move_left(void);
  void move_right(void);
  void blink(void);

protected:
  input_ctrl(void) = delete;
  input_ctrl(const input_ctrl&) = delete;
  input_ctrl(input_ctrl&&) = delete;
  input_ctrl& operator=(const input_ctrl&) = delete;
  input_ctrl& operator=(input_ctrl&&) = delete;

  std::vector<uint> SizeOfSymbols {};

  uint row_position = 0;
  uint row_limit = 128;
  uint row_size = 0;
  float_color ColorBgOn {0.0f, 0.8f, 0.0f, 0.6f};
  bool visible = false;
};


///
/// \brief The element struct
///
struct element {
  std::vector<size_t> FacesID {};          // Список ID поверхностей в массиве SymbolsBuffer
  func_ptr caller = nullptr;               // Адрес функции, вызываемой по нажатию
  STATES state = ST_NORMAL;                // Текущее состояние
  confines Margins {};                     // Внешние границы элемента для обработки событий курсора
  ELEMENT_TYPES element_type = GUI_BUTTON; // тип графического элемента
  uint id = 0;
};


///
/// \brief Структура для хранения параметров создаваемых элементов интерфейса.
///
/// \details
/// Для того, чтобы корректно распределить все элементы графического интерфейса в окне,
/// необходимо знать их общее количество. Поэтому все данные создаваемых элементов
/// вначале собираются в общий массив, потом выполняется расчет их положения в окне,
/// и уже после этого элементы создаются.
///
struct params {
  std::string Label {};
  func_ptr caller = nullptr;
  STATES state = ST_NORMAL;                // Текущее состояние
  bool dependant = false;                  // Зависит ли состояние элемента от других элементов
  uint left = 0;
  uint top = 0;
};


///
/// \brief The gui_group class
///
class gui_group
{
protected:
  std::vector<params> Params {};    // Данные всех элементов группы

public:
  gui_group(void) = default;
  virtual ~gui_group(void) = default;

  void append(const std::string& newLabel, func_ptr callback = nullptr,
              STATES state = ST_NORMAL, bool dependant = false);

  virtual void align(void) = 0;
  virtual void make(void) = 0;
};


///
/// \brief The buttons_group class
///
class buttons: public gui_group
{
public:
  virtual void align(void);
  virtual void make(void);
};


///
/// \brief The rows class
///
class rows: public gui_group
{
public:
  virtual void make(void);
  virtual void align(void);
};


///
/// \brief The gui class
///
class gui: public interface_gl_context
{
  public:
    gui(void);
    ~gui(void);

    static bool open;
    virtual void event_resize(int width, int height);
    virtual void event_cursor(double x, double y);                                // x, y
    virtual void event_mouse_btns(int _button, int _action, int _mods);           // _button, _action, _mods
    virtual void event_keyboard(int _key, int _scancode, int _action, int _mods); // _key, _scancode, _action, _mods
    virtual void event_focus_lost();
    virtual void event_character(uint ch);
    virtual void event_error(const char* message);
    virtual void event_reposition(int left, int top);
    virtual void event_close(void);

    static std::shared_ptr<trgl> OGLContext;   // OpenGL контекст окна приложения

    bool render(void);

  private:
    // Запретить копирование и перенос экземпляра класса
    gui(const gui&) = delete;
    gui& operator=(const gui&) = delete;
    gui(gui&&) = delete;
    gui& operator=(gui&&) = delete;

    int FPS = 0;                   // частота кадров
    static GLsizei fps_uv_data;    // смещение данных FPS в буфере UV
    static std::string map_current;
    static std::string StringBuffer;   // строка ввода пользователя
    static func_ptr last_menu;

    static std::unique_ptr<space_3d> Space3d;    // = nullptr;
    std::unique_ptr<glsl> Program2d = nullptr;   // построение 2D элементов

    static bool RUN_3D;
    GLuint vao_fbuf = 0;                         // Рендер текстуры фрейм-буфера
    GLuint vao_2d =   0;                         // Рендер элементов меню и HUD
    static glm::vec3 Cursor3D;                   // положение и размер прицела

    static std::unique_ptr<input_ctrl> InputCursor;      // Текстовый курсор ввода пользователя
    static std::unique_ptr<glsl> ProgramFrBuf;           // Шейдерная программа GUI

    void program_2d_init(void);
    void program_fbuf_init(void);
    void remove_map(void);
    void calc_fps(void);
    void hud_update(void);

    void callback_render(void);
    void update_input(void);

    static void map_close(void);
    static void clear(void);
    static void screen_start(uint);
    static void screen_config(uint);
    static void screen_map_select(uint);
    static void screen_map_new(uint);
    static void screen_pause(uint);
    static void hud_enable(void);

    static void title(const std::string& Label);
    static void map_create(uint);
    static void map_open(uint);
    static void text_append(const layout& L, const std::vector<std::string>& Text, uint kerning);
    static void element_move(element& Elm, int x, int y);
    static void element_set_state(element& Button, STATES s);
    static void select_row(uint id);
    static void close_map(uint);
    static void mode_3d(uint);
    static void close(uint) { open = false; }

    static void mode_2d(void);
};

}

#endif // GUI_HPP
