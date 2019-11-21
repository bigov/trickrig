#ifndef WGLFW_HPP
#define WGLFW_HPP

#include "io.hpp"
#include "db.hpp"

namespace tr
{

class wglfw
{
  static std::string title;

  public:
      wglfw(int width=0, int height=0, int min_w=0, int min_h=0, int left=0, int top=0);
      ~wglfw(void);

      // Запретить копирование объекта
      wglfw(const wglfw&) = delete;
      wglfw& operator=(const wglfw&) = delete;

      // Запретить перенос объекта
      wglfw(wglfw&&) = delete;
      wglfw& operator=(wglfw&&) = delete;

      void swap_buffers(void);
      void cursor_hide(void);
      void cursor_restore(void);

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
      static bool sight_mode;
      static double half_w; // середина окна по X
      static double half_h; // середина окна по Y

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
