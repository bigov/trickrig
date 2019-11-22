#ifndef WGLFW_HPP
#define WGLFW_HPP

#include "io.hpp"

namespace tr
{

class wglfw
{
  static std::string title;

  public:
      wglfw(void);
      ~wglfw(void);

      // Запретить копирование объекта
      wglfw(const wglfw&) = delete;
      wglfw& operator=(const wglfw&) = delete;

      // Запретить перенос объекта
      wglfw(wglfw&&) = delete;
      wglfw& operator=(wglfw&&) = delete;

      void set_window(u_int width=10, u_int height=10, u_int min_w=0,
                      u_int min_h=0, u_int left=0, u_int top=0);
      void swap_buffers(void);
      void cursor_hide(void);
      void cursor_restore(void);
      void set_cursor_pos(double x, double y);
      void get_frame_buffer_size(int* width, int* height);

      void set_error_observer(interface_gl_context& ref);    // отслеживание ошибок
      void set_cursor_observer(interface_gl_context& ref);   // курсор мыши в окне
      void set_button_observer(interface_gl_context& ref);   // кнопки мыши
      void set_keyboard_observer(interface_gl_context& ref); // клавиши клавиатуры
      void set_position_observer(interface_gl_context& ref); // положение окна
      void add_size_observer(interface_gl_context& ref);     // размер окна
      void set_char_observer(interface_gl_context& ref);     // ввод текста (символ)
      void set_close_observer(interface_gl_context& ref);    // закрытие окна

    private:
      static GLFWwindow* win_ptr;

      static interface_gl_context* error_observer;
      static interface_gl_context* cursor_observer;
      static interface_gl_context* button_observer;
      static interface_gl_context* keyboard_observer;
      static interface_gl_context* position_observer;
      static std::list<interface_gl_context*> size_observers;
      static interface_gl_context* char_observer;
      static interface_gl_context* close_observer;

      static void callback_error(int error_id, const char* description);
      static void callback_cursor(GLFWwindow* window, double xpos, double ypos);
      static void callback_button(GLFWwindow* window, int button, int action, int mods);
      static void callback_keyboard(GLFWwindow*, int key, int scancode, int action, int mods);
      static void callback_position(GLFWwindow*, int, int);
      static void callback_size(GLFWwindow*, int, int);
      static void callback_char(GLFWwindow*, unsigned int);
      static void callback_close(GLFWwindow*);

  };

} //namespace tr

#endif //WGLFW_HPP
