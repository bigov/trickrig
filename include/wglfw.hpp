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

    private:
      GLFWwindow * win_ptr = nullptr;

      // переменная для запроса положения курсора в окне
      double mouse_x = 0.0,
             mouse_y = 0.0;

      // TODO: setup by Config
      static int k_FRONT;
      static int k_BACK;
      static int k_UP;
      static int k_DOWN;
      static int k_RIGHT;
      static int k_LEFT;

      wglfw(const tr::wglfw &);
      wglfw operator=(const tr::wglfw &);

      static void error_callback(int error_id, const char* description);

      static void cursor_position_callback(
          GLFWwindow* window, double xpos, double ypos);

      static void mouse_button_callback(
        GLFWwindow* window, int button, int action, int mods);

      static void key_callback(
        GLFWwindow* window, int key, int scancode, int action, int mods);

      static void window_pos_callback(GLFWwindow*, int, int);
      static void framebuffer_size_callback(GLFWwindow*, int, int);
      static void character_callback(GLFWwindow*, unsigned int);

  };
} //namespace tr

#endif //WGLFW_HPP
