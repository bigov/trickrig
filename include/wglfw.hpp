#ifndef WGLFW_HPP
#define WGLFW_HPP

#include "io.hpp"

namespace tr
{

class wglfw
{
  static std::string title;

  public:
      wglfw() = default;
      ~wglfw(void);

      // Запретить копирование объекта
      wglfw(const wglfw&) = delete;
      wglfw& operator=(const wglfw&) = delete;

      // Запретить перенос объекта
      wglfw(wglfw&&) = delete;
      wglfw& operator=(wglfw&&) = delete;

      void init(u_int width=0, u_int height=0, u_int min_w=0, u_int min_h=0, u_int left=0, u_int top=0);
      void swap_buffers(void);
      void cursor_hide(void);
      void cursor_restore(void);
      void set_cursor_pos(double x, double y);

      void set_error_observer(IWindowInput& ref);    // отслеживание ошибок
      void set_cursor_observer(IWindowInput& ref);   // курсор мыши в окне
      void set_button_observer(IWindowInput& ref);   // кнопки мыши
      void set_keyboard_observer(IWindowInput& ref); // клавиши клавиатуры
      void set_position_observer(IWindowInput& ref); // положение окна
      void add_size_observer(IWindowInput& ref);     // размер окна
      void set_char_observer(IWindowInput& ref);     // ввод текста (символ)
      void set_close_observer(IWindowInput& ref);    // закрытие окна

    private:
      static GLFWwindow* win_ptr;

      static IWindowInput* error_observer;
      static IWindowInput* cursor_observer;
      static IWindowInput* button_observer;
      static IWindowInput* keyboard_observer;
      static IWindowInput* position_observer;
      static std::list<IWindowInput*> size_observers;
      static IWindowInput* char_observer;
      static IWindowInput* close_observer;

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
