//============================================================================
//
// file win_glfw.cpp
//
// интерфейс к библиотеке GLFW
//
//============================================================================
#include "glfw.hpp"

namespace tr
{
  evInput window_glfw::keys = {0.0, 0.0, 0, 0, 0, 0, 0, 0, 120};
  std::string window_glfw::title = "TrickRig: v.development";
  bool window_glfw::cursor_is_captured = false;
  double window_glfw::x0 = 0;
  double window_glfw::y0 = 0;

  ////////
  // Errors callback
  //
  void window_glfw::error_callback(int error, const char* description)
  {
    info("GLFW error " + std::to_string(error) + ": " + description);
    return;
  }

  ////////
  //
  //
  void window_glfw::cursor_grab(GLFWwindow * window)
  {
    cursor_is_captured = true;
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_HIDDEN);
    glfwSetCursorPos(window, x0, y0);
    return;
  }

  ////////
  //
  //
  void window_glfw::cursor_free(GLFWwindow * window)
  {
    cursor_is_captured = false;
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
    return;
  }

  ////////
  // Mouse keys callback
  //
  void window_glfw::mouse_button_callback(
    GLFWwindow* window, int button, int action, int mods)
  {
    if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_PRESS)
      if (!cursor_is_captured) cursor_grab(window);

    keys.mouse_mods = mods;
    return;
  }

  ////////
  // Keys events callback
  //
  void window_glfw::key_callback(GLFWwindow* window, int key, int scancode,
    int action, int mods)
  {
    if (key == GLFW_KEY_ESCAPE && action == GLFW_RELEASE)
    {
      if (cursor_is_captured)
      {
        keys.fb = 0;
        keys.ud = 0;
        keys.rl = 0;
        keys.dx = 0;
        keys.dy = 0;
        cursor_free(window);
      }
      else glfwSetWindowShouldClose(window, true);
    }

    keys.key_mods = mods;
    keys.key_scancode = scancode;
    return;
  }

  ////////
  // Опрос состояния клавиш управления
  //
  void window_glfw::check_keys_state(void)
  {
    keys.fb = glfwGetKey(pWin, k_FRONT) - glfwGetKey(pWin, k_BACK);
    keys.ud = glfwGetKey(pWin, k_DOWN)  - glfwGetKey(pWin, k_UP);
    keys.rl = glfwGetKey(pWin, k_RIGHT) - glfwGetKey(pWin, k_LEFT);
    return;
  }

  ////////
  // GLFW framebuffer callback resize
  //
  void window_glfw::framebuffer_size_callback(GLFWwindow * window,
    int width, int height)
  {
    x0 = static_cast<double>(width/2);
    y0 = static_cast<double>(height/2);

    if (!window) ERR("Error on call GLFW framebuffer_size_callback.");
    glViewport(0, 0, width, height);
    tr::config::set_size(width, height);
    return;
  }

  ////////
  // Создание нового окна с обработчиками ввода и настройка контекста отображения OpenGL
  //
  window_glfw::window_glfw(void)
  {
    glfwSetErrorCallback(error_callback);
    if (!glfwInit()) ERR("Error init GLFW lib.");

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, 0);
    glfwWindowHint(GLFW_OPENGL_DEBUG_CONTEXT, 1);

    pWin = glfwCreateWindow(tr::Cfg.gui.w, tr::Cfg.gui.h, title.c_str(), NULL, nullptr);
    if (nullptr == pWin) ERR("Creating Window fail.");
    glfwMakeContextCurrent(pWin);
    glfwSwapInterval(0);
    glfwSetKeyCallback(pWin, key_callback);
    glfwSetMouseButtonCallback(pWin, mouse_button_callback);
    glfwSetFramebufferSizeCallback(pWin, framebuffer_size_callback);

    if(!ogl_LoadFunctions())  ERR("Can't load OpenGl finctions");

    x0 = static_cast<double>(tr::Cfg.gui.w)/2.0;
    y0 = static_cast<double>(tr::Cfg.gui.h)/2.0;

    return;
  }

  ////////
  // Destructor
  //
  window_glfw::~window_glfw()
  {
    glfwSetInputMode(pWin, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
    glfwTerminate();
    return;
  }

  ////////
  // Опрос положения указателя мыши и возврат его в центр окна
  // производится в конце отрисовки каждого кадра
  //
  void window_glfw::check_mouse_pos(void)
  {
    glfwGetCursorPos(pWin, &xpos, &ypos);
    glfwSetCursorPos(pWin, x0, y0);
    keys.dx = static_cast<float>(xpos - x0);
    keys.dy = static_cast<float>(ypos - y0);
    return;
  }


  ////////
  // Show content
  //
  void window_glfw::show(tr::scene & space)
  {
    glfwSetInputMode(pWin, GLFW_STICKY_KEYS, 0);

    int fps = 0;
    std::chrono::seconds one_second(1);
    std::chrono::time_point<std::chrono::system_clock> t_start, t_frame;
    std::string win_title = title + std::to_string(fps);

    t_start = std::chrono::system_clock::now();
    while (!glfwWindowShouldClose(pWin))
    {
      fps++;
      t_frame = std::chrono::system_clock::now();
      if (t_frame - t_start >= one_second)
      {
        t_start = std::chrono::system_clock::now();
        keys.fps = fps;
        fps = 0;
      }

      if (cursor_is_captured)
      {
        check_mouse_pos();
        check_keys_state();
      }

      space.draw(keys);

      glfwSwapBuffers(pWin);
      glfwPollEvents();
    }
    glfwDestroyWindow(pWin);
    return;
  }

} //namespace tr
