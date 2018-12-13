#ifndef WINGL_HPP
#define WINGL_HPP

#include "config.hpp"
#include "main.hpp"
#include "scene.hpp"
#include "io.hpp"
#include "GLFW/glfw3.h"

namespace tr
{
  class wglfw
  {
    static evInput keys;
    static std::string title;

    public:
      wglfw(void);
      ~wglfw(void);
      void show(tr::scene&);

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

      void set_cursor(void);
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

#endif // WINGL_HPP
