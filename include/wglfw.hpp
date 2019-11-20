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
      void swap_buffers(void);
      void cursor_hide(void);
      void cursor_restore(void);

      void set_win_observer(IWindowInput& ref); // добавить наблюдателя окна
      void set_char_observer(IWindowInput& ref); // добавить наблюдателя ввода
      void set_size_observer(IWindowInput& ref); // добавить наблюдателя размера

      // Запретить копирование объекта
      wglfw(const wglfw&) = delete;
      wglfw& operator=(const wglfw&) = delete;

      // Запретить перенос объекта
      wglfw(wglfw&&) = delete;
      wglfw& operator=(wglfw&&) = delete;

    private:
      static GLFWwindow* win_ptr;
      static IWindowInput* win_observer;
      static IWindowInput* char_observer;
      static IWindowInput* size_observer;

      static double half_w;   // середина окна по X
      static double half_h;   // середина окна по Y

      static void error_callback(int error_id, const char* description);

      // движение указателя мыши в окне
      static void cursor_position_callback(
          GLFWwindow* window, double xpos, double ypos);

      // смещение прицела в режиме 3D
      static void sight_position_callback(
          GLFWwindow* window, double xpos, double ypos);

      static void mouse_button_callback(
        GLFWwindow* window, int button, int action, int mods);

      static void keyboard_callback(GLFWwindow*, int key, int scancode, int action, int mods);

      static void reposition_callback(GLFWwindow*, int, int);

      static void resize_callback(GLFWwindow*, int, int);

      static void character_callback(GLFWwindow*, unsigned int);

      static void window_close_callback(GLFWwindow*);

  };
} //namespace tr

#endif //WGLFW_HPP
