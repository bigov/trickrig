#include "GLFW/glfw3.h"

namespace tr {

// Интерфейс графического окна для обмена данными с OpenGL контентом
class IWindowInput
{
public:
    IWindowInput(void) = default;
    virtual ~IWindowInput(void) = default;

    virtual void error_event(const char*) {}
    virtual void character_event(unsigned int) {}
    virtual void mouse_event(int, int, int) {}
    virtual void keyboard_event(int, int, int, int) {}
    virtual void reposition_event(int, int) {}
    virtual void resize_event(GLsizei, GLsizei) {}
    virtual void cursor_event(double, double) {}
    virtual void close_event(void) {}
};

}
