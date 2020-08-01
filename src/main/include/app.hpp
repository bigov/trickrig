#ifndef APP_HPP
#define APP_HPP

#include "main.hpp"
#include "trgl.hpp"
#include "gui.hpp"
#include "space.hpp"

namespace tr {

struct map
{
  map(const std::string &f, const std::string &n): Folder(f), Name(n) {}
  std::string Folder;
  std::string Name;
};


class app: public interface_gl_context
{
  public:
    app(void);
    ~app(void);

    // Запретить копирование и перенос экземпляра класса
    app(const app&) = delete;
    app& operator=(const app&) = delete;
    app(app&&) = delete;
    app& operator=(app&&) = delete;

    void show(void);

    virtual void reposition_event(int left, int top);
    virtual void resize_event(int width, int height);
    virtual void character_event(uint ch);
    virtual void cursor_event(double x, double y);
    virtual void close_event(void);
    virtual void error_event(const char* message);
    virtual void mouse_event(int _button, int _action, int _mods);
    virtual void keyboard_event(int _key, int _scancode, int _action, int _mods);
    virtual void focus_lost_event();

  private:
    enum MENU_MODES {    // режимы окна
      SCREEN_START,   // начальное меню
      SCREEN_LSELECT, // выбор игры
      SCREEN_CREATE,  // создание нового района
      SCREEN_CONFIG,  // настройки
    };

    static bool RUN_3D;

    std::shared_ptr<trgl> GLContext = nullptr;
    static std::unique_ptr<glsl> ShowScene; // Шейдерная программа GUI
    static glm::vec3 Cursor3D;               // положение и размер прицела

    int scancode = -1;
    int mods = -1;
    int action = -1;
    int key = -1;

    static layout Layout;                    // положение окна и размеры
    float aspect = 1.0f;                     // соотношение размеров окна
    static std::unique_ptr<space_3d> Space3d;     // = nullptr;
    static std::unique_ptr<gui> AppGUI; // = nullptr

    bool text_mode = false;                  // режим ввода текста
    std::string StringBuffer {};             // строка ввода пользователя
    static double mouse_x;                    // позиция указателя относительно левой границы
    static double mouse_y;                    // позиция указателя относительно верхней границы

    uchar_color bgColor {0xE0, 0xE0, 0xE0, 0xC0}; // цвет фона неактивного окна
    static menu_screen MenuOnImage;            // GUI окна приложения
    static uchar_color color_title;         // фон заголовка
    static int mouse_left;                  // нажатие на левую кнопку мыши

    static GLuint texture_gui;              // id тектуры HUD

    static std::vector<map> Maps;          // список карт
    static MENU_MODES MenuMode;              // режим окна приложения
    static size_t row_selected;            // какая строка выбрана

    static GLuint vao2d;
    std::chrono::time_point<std::chrono::system_clock> TimeStart;

    static void mode_3d(void);
    static void mode_2d(void);
    void cursor_text_row(const atlas& _Fn, image &_Dst, size_t position);
    void title(const std::string& title);
    void input_text_line(const atlas& _Fn);
    static void row_text(size_t id, uint x, uint y, uint w, uint h, const std::string &);
    void menu_map_create(void);
    static void menu_select(void);
    static void menu_start(void);
    static void menu_config(void);
    void window_frame_render(void);
    void cancel(void);
    void create_map(void);
    void remove_map(void);
    static void map_open(uint map_id);
    void layout_set(const layout &L);
    static void update_gui_image(void);
    static void app_close(void);
};

} //tr
#endif // GUI_HPP
