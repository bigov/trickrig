#ifndef INTERFACE_GL_CONTEXT
#define INTERFACE_GL_CONTEXT

#define GLFW_INCLUDE_NONE
#include "GLFW/glfw3.h"

namespace tr {

// Интерфейс графического окна для обмена данными с OpenGL контентом
class interface_gl_context
{
public:
    interface_gl_context(void) = default;
    virtual ~interface_gl_context(void) = default;

    virtual void event_error(const char*) {}
    virtual void event_character(unsigned int) {}
    virtual void event_mouse_btns(int, int, int) {}
    virtual void event_keyboard(int, int, int, int) {}
    virtual void event_reposition(int, int) {}
    virtual void event_resize(int, int) {}
    virtual void event_cursor(double, double) {}
    virtual void event_close(void) {}
    virtual void event_focus_lost(void) {}
};

}      // namespace tr
#endif //INTERFACE_GL_CONTEXT
