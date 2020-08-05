#ifndef APP_HPP
#define APP_HPP

#include "trgl.hpp"
#include "gui.hpp"

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
    virtual void character_event(uint ch);
    virtual void close_event(void);
    virtual void error_event(const char* message);

  private:
    enum MENU_MODES {    // режимы окна
      SCREEN_START,   // начальное меню
      SCREEN_LSELECT, // выбор игры
      SCREEN_CREATE,  // создание нового района
      SCREEN_CONFIG,  // настройки
    };

    std::shared_ptr<trgl> GLContext = nullptr;

    int scancode = -1;
    int mods = -1;
    int action = -1;
    int key = -1;

    static layout Layout;                   // положение окна и размеры
    float aspect = 1.0f;                    // соотношение размеров окна
    static std::unique_ptr<gui> AppGUI;     // = nullptr

    bool text_mode = false;                 // режим ввода текста
    std::string StringBuffer {};            // строка ввода пользователя
    static double mouse_x;                  // позиция указателя относительно левой границы
    static double mouse_y;                  // позиция указателя относительно верхней границы

    uchar_color bgColor {0xE0, 0xE0, 0xE0, 0xC0}; // цвет фона неактивного окна
    static uchar_color color_title;         // фон заголовка
    static int mouse_left;                  // нажатие на левую кнопку мыши

    static GLuint texture_gui;              // id тектуры HUD

    static std::vector<map> Maps;           // список карт
    static MENU_MODES MenuMode;             // режим окна приложения
    static size_t row_selected;             // какая строка выбрана

    std::chrono::time_point<std::chrono::system_clock> TimeStart;

    void cursor_text_row(const atlas& _Fn, image &_Dst, size_t position);
    void menu_map_create(void);
    void create_map(void);
    void remove_map(void);
};

} //tr
#endif // GUI_HPP
