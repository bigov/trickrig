#ifndef INTERFACE_GL_CONTENT
#define INTERFACE_GL_CONTENT

#include "GLFW/glfw3.h"

namespace tr {

// Интерфейс графического окна для обмена данными с OpenGL контентом
class interface_gl_context
{
public:
    interface_gl_context(void) = default;
    virtual ~interface_gl_context(void) = default;

    virtual void error_event(const char*) {}
    virtual void character_event(unsigned int) {}
    virtual void mouse_event(int, int, int) {}
    virtual void keyboard_event(int, int, int, int) {}
    virtual void reposition_event(int, int) {}
    virtual void resize_event(int, int) {}
    virtual void cursor_event(double, double) {}
    virtual void close_event(void) {}
    virtual void focus_lost_event(void) {}

};

}

#endif
