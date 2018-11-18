#ifndef __WIN_GLFW_HPP__
#define __WIN_GLFW_HPP__

#include "config.hpp"
#include "main.hpp"
#include "scene.hpp"
#include "io.hpp"
#include "GLFW/glfw3.h"

namespace tr
{
  class glfw_wr
  {
    static evInput keys;
    static std::string title;
    //static int win_center_x, win_center_y; // центр окна

    public:
      glfw_wr(void);
      ~glfw_wr(void);
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

      glfw_wr(const tr::glfw_wr &);
      glfw_wr operator=(const tr::glfw_wr &);
      static void scene_open(GLFWwindow*);
      static void cursor_free(GLFWwindow*);

      static void error_callback(int error_id, const char* description);

      static void cursor_position_callback(
          GLFWwindow* window, double xpos, double ypos);

      static void mouse_button_callback(
        GLFWwindow* window, int button, int action, int mods);

      static void key_callback(
        GLFWwindow* window, int key, int scancode, int action, int mods);

      static void escape_key(GLFWwindow* window);
      static void window_pos_callback(GLFWwindow*, int, int);
      static void framebuffer_size_callback(GLFWwindow*, int, int);

  };
} //namespace tr

#endif //_WIN_GLFW_HPP_
