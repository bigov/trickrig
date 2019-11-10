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
      wglfw(void);
      ~wglfw(void);
      void swap_buffers(void);
      void cursor_hide(void);
      void cursor_restore(void);

      void append(IUserInput& ref);  // добавить наблюдателя
      void remove(IUserInput& ref);  // удалить наблюдателя

      // Запретить копирование объекта
      wglfw(const wglfw&) = delete;
      wglfw& operator=(const wglfw&) = delete;

      // Запретить перенос объекта
      wglfw(wglfw&&) = delete;
      wglfw& operator=(wglfw&&) = delete;

    private:
      GLFWwindow * win_ptr = nullptr;
      static std::list<IUserInput*> _observers;

      static void error_callback(int error_id, const char* description);

      static void cursor_position_callback(
          GLFWwindow* window, double xpos, double ypos);

      static void mouse_button_callback(
        GLFWwindow* window, int button, int action, int mods);

      static void key_callback(GLFWwindow*, int key, int scancode, int action, int mods);

      static void window_pos_callback(GLFWwindow*, int, int);

      static void framebuffer_size_callback(GLFWwindow*, int, int);

      static void character_callback(GLFWwindow*, unsigned int);

  };
} //namespace tr

#endif //WGLFW_HPP
