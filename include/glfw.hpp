#ifndef __WIN_GLFW_HPP__
#define __WIN_GLFW_HPP__

#include "config.hpp"
#include "main.hpp"
#include "scene.hpp"
#include "io.hpp"

namespace tr
{

  class window_glfw
  {
    static evInput keys;
    static std::string title;
    static bool cursor_is_captured;
    static double x0, y0; // центр окна

    public:
      window_glfw(void);
      ~window_glfw(void);
      void show(tr::scene&);
      void check_mouse_pos(void);
      void check_keys_state(void);

    private:
      // переменная для запроса положения курсора в окне
      double xpos = 0.0,
             ypos = 0.0;

      // TODO: setup by Config
      int k_FRONT = GLFW_KEY_W;
      int k_BACK  = GLFW_KEY_S;
      int k_UP    = GLFW_KEY_LEFT_SHIFT;
      int k_DOWN  = GLFW_KEY_SPACE;
      int k_RIGHT = GLFW_KEY_D;
      int k_LEFT  = GLFW_KEY_A;

      window_glfw(const tr::window_glfw &);
      window_glfw operator=(const tr::window_glfw &);
      static void cursor_grab(GLFWwindow*);
      static void cursor_free(GLFWwindow*);

      static void error_callback(int error_id, const char* description);

      static void mouse_button_callback(
        GLFWwindow* window, int button, int action, int mods);

      static void key_callback(
        GLFWwindow* window, int key, int scancode, int action, int mods);

      static void framebuffer_size_callback(
        GLFWwindow* window, int width, int height);

      GLFWwindow * pWin = nullptr;
  };
} //namespace tr

#endif //_WIN_GLFW_HPP_
