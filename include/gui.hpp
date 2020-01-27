#ifndef GUI_HPP
#define GUI_HPP

#include "main.hpp"
#include "wglfw.hpp"
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


class gui: public interface_gl_context
{
  public:
    gui(void);
    ~gui(void);

    // Запретить копирование и перенос экземпляра класса
    gui(const gui&) = delete;
    gui& operator=(const gui&) = delete;
    gui(gui&&) = delete;
    gui& operator=(gui&&) = delete;

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
    enum GUI_MODES {    // режимы окна
      GUI_3D_MODE,      // основной режим - без шторки
      GUI_MENU_START,   // начальное меню
      GUI_MENU_LSELECT, // выбор игры
      GUI_MENU_CREATE,  // создание нового района
      GUI_MENU_CONFIG,  // настройки
    };
    enum ELEMENT_ID {   // Идентификаторы кнопок GIU
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
    struct map{
        map(const std::string &f, const std::string &n): Folder(f), Name(n){}
        std::string Folder;
        std::string Name;
    };
    std::shared_ptr<wglfw> GLContext = nullptr;
    std::unique_ptr<glsl> Program2d = nullptr;           // Шейдерная программа GUI
    glm::vec3 Cursor3D = { 200.f, 200.f, 0.f };          // положение и размер прицела
    const uint BUTTTON_WIDTH = 120;                      // ширина кнопки GUI
    const uint BUTTTON_HEIGHT = 36;                      // высота кнопки GUI
    const uint MIN_GUI_WIDTH = (BUTTTON_WIDTH + 16) * 4; // минимально допустимая ширина окна
    const uint MIN_GUI_HEIGHT = BUTTTON_HEIGHT * 4 + 8;  // минимально допустимая высота окна

    int scancode = -1;
    int mods = -1;
    int action = -1;
    int key = -1;

    bool is_open = true;                     // состояние окна
    layout Layout {400, 400, 0, 0};          // размеры и положение
    float aspect = 1.0f;                     // соотношение размеров окна
    std::unique_ptr<space> Space = nullptr;

    bool text_mode = false;                  // режим ввода текста
    std::string StringBuffer {};             // строка ввода пользователя
    double mouse_x = 0.0;                    // позиция указателя относительно левой границы
    double mouse_y = 0.0;                    // позиция указателя относительно верхней границы

    px bg      {0xE0, 0xE0, 0xE0, 0xC0};     // фон заполнения неактивного окна
    img ImgGUI { 0, 0 };                     // GUI текстура окна приложения
    px color_title {0xFF, 0xFF, 0xDD, 0xFF}; // фон заголовка
    int mouse_left = EMPTY;                 // нажатие на левую кнопку мыши

    GLuint texture_gui = 0;                  // id тектуры HUD

    std::vector<map> Maps {};           // список карт
    GUI_MODES GuiMode = GUI_MENU_START; // режим окна приложения
    ELEMENT_ID element_over = NONE;     // Над какой GIU кнопкой курсор
    size_t row_selected = 0;            // какая строка выбрана

    GLuint vao_quad_id  = 0;
    std::chrono::time_point<std::chrono::system_clock> TimeStart;

    void button(ELEMENT_ID id, ulong x, ulong y, const std::string& Name,
                bool button_is_active = true );
    void button_make_body(img &Data, BUTTON_STATE);
    void cursor_text_row(const img &_Fn, img &_Dst, size_t position);
    void title(const std::string& title);
    void input_text_line(const img &_Fn);
    void row_text(size_t id, uint x, uint y, uint w, uint h, const std::string &);
    void select_list(uint x, uint y, uint w, uint h);
    void menu_build(void);
    void menu_map_create(void);
    void menu_map_select(void);
    void menu_start(void);
    void menu_config(void);
    void button_click(ELEMENT_ID);
    void cancel(void);
    void screen_render(void);
    void create_map(void);
    void remove_map(void);
    void layout_set(const layout &L);

};

} //tr
#endif // GUI_HPP
