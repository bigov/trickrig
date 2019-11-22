#ifndef WINDOW_PARAMETERS
#define WINDOW_PARAMETERS

#include "io.hpp"
#include "i_win.hpp"

// Параметры и режимы окна приложения
namespace tr {

class win_params: public interface_gl_context
{
  public:
    win_params(void) = default;

    // Запретить копирование и перенос экземпляров класса
    win_params(const win_params&) = delete;
    win_params& operator=(const win_params&) = delete;
    win_params(win_params&&) = delete;
    win_params& operator=(win_params&&) = delete;
    img* pWinGui = nullptr;                   // текстура GUI окна
    double xpos = 0.0;             // позиция указателя относительно левой границы
    double ypos = 0.0;             // позиция указателя относительно верхней границы
    glm::vec3 Sight = { 200.f, 200.f, .0f }; // положение и размер прицела
    float aspect = 1.0f;  // соотношение размеров окна

    layout Layout {400, 400, 0, 0};           // размеры и положение
    int scancode = -1;
    int mods = -1;
    int mouse = -1;
    int action = -1;
    int key = -1;

    unsigned int btn_w = 120;                 // ширина кнопки GUI
    unsigned int btn_h = 36;                  // высота кнопки GUI
    unsigned int minwidth = (btn_w + 16) * 4; // минимально допустимая ширина окна
    unsigned int minheight = btn_h * 4 + 8;   // минимально допустимая высота окна
    bool is_open = true;  // состояние окна

    virtual void error_event(const char* message);
    virtual void mouse_event(int _button, int _action, int _mods);
    virtual void keyboard_event(int _key, int _scancode, int _action, int _mods);
    virtual void reposition_event(int left, int top);
    virtual void resize_event(int width, int height);
    virtual void cursor_event(double x, double y);
    virtual void close_event(void);

    void layout_set(const layout &L);
};

}
#endif
