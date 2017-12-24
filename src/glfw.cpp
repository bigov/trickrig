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
  double window_glfw::win_center_x = 0;
  double window_glfw::win_center_y = 0;
  //glm::vec3 ViewFrom = {};      // 3D координаты точка обзора

  opengl_window_params GlWin = {};

  //## Errors callback
  void window_glfw::error_callback(int error, const char* description)
  {
    info("GLFW error " + std::to_string(error) + ": " + description);
    return;
  }

  //##
  void window_glfw::cursor_grab(GLFWwindow * window)
  {
    cursor_is_captured = true;
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_HIDDEN);
    glfwSetCursorPos(window, win_center_x, win_center_y);
    return;
  }

  //##
  void window_glfw::cursor_free(GLFWwindow * window)
  {
    cursor_is_captured = false;
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
    return;
  }

  //##
  void window_glfw::mouse_button_callback(
    GLFWwindow* window, int button, int action, int mods)
  {
    if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_PRESS)
      if (!cursor_is_captured) cursor_grab(window);

    keys.mouse_mods = mods;
    return;
  }

  //## Keys events callback
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

  //## Опрос состояния клавиш управления
  void window_glfw::check_keys_state(void)
  {
    keys.fb = glfwGetKey(win_ptr, k_FRONT)
            - glfwGetKey(win_ptr, k_BACK);
    keys.ud = glfwGetKey(win_ptr, k_DOWN)
            - glfwGetKey(win_ptr, k_UP);
    keys.rl = glfwGetKey(win_ptr, k_RIGHT)
            - glfwGetKey(win_ptr, k_LEFT);
    return;
  }

  //## GLFW window moving callback
  void window_glfw::window_pos_callback(GLFWwindow * window,
    int left, int top)
  {
    if (!window) ERR("Error on call GLFW window_pos_callback.");
    tr::GlWin.left = left;
    tr::GlWin.top = top;
    return;
  }

  //## GLFW framebuffer callback resize
  void window_glfw::framebuffer_size_callback(GLFWwindow * window,
    int width, int height)
  {
    win_center_x = static_cast<double>(width / 2);
    win_center_y = static_cast<double>(height / 2);

    if (!window) ERR("Error on call GLFW framebuffer_size_callback.");
    glViewport(0, 0, width, height);

    if(width < 64)  width  = 64;
    if(height < 48) height = 48;
    tr::GlWin.width  = width;
    tr::GlWin.height = height;

    // пересчет координат курсора
    tr::GlWin.Cursor.x = static_cast<float>(tr::GlWin.width/2) + 0.5;
    tr::GlWin.Cursor.y = static_cast<float>(tr::GlWin.height/2) + 0.5;

    // пересчет матрицы проекции
    tr::GlWin.aspect = static_cast<float>(tr::GlWin.width)
                     / static_cast<float>(tr::GlWin.height);
    tr::MatProjection = glm::perspective(1.118f, tr::GlWin.aspect, 0.01f, 1000.0f);

    // Перестрока фреймбуфера
    glBindFramebuffer(GL_FRAMEBUFFER, Eye.frame_buf);

    // настройка размера текстуры
    glBindTexture(GL_TEXTURE_2D, Eye.texco_buf);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, tr::GlWin.width, tr::GlWin.height, 0,
      GL_RGBA, GL_UNSIGNED_BYTE, NULL);

    // настройка размера рендербуфера
    glBindRenderbuffer(GL_RENDERBUFFER, Eye.rendr_buf);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8,
      tr::GlWin.width, tr::GlWin.height);

    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    return;
  }

  //## Создание нового окна с обработчиками ввода и настройка контекста отображения OpenGL
  window_glfw::window_glfw(void)
  {
    glfwSetErrorCallback(error_callback);
    if (!glfwInit()) ERR("Error init GLFW lib.");

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 2);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, 0);
    glfwWindowHint(GLFW_OPENGL_DEBUG_CONTEXT, 1);

    // Начальные настройки камеры вида и размера окна
    tr::Eye.ViewFrom.x = std::stof(tr::Cfg.get(VIEW_FROM_X));
    tr::Eye.ViewFrom.y = std::stof(tr::Cfg.get(VIEW_FROM_Y));
    tr::Eye.ViewFrom.z = std::stof(tr::Cfg.get(VIEW_FROM_Z));
    tr::GlWin.width = std::stoi(tr::Cfg.get(WINDOW_WIDTH));
    tr::GlWin.height = std::stoi(tr::Cfg.get(WINDOW_HEIGHT));

    tr::GlWin.top = std::stoi(tr::Cfg.get(WINDOW_TOP));
    tr::GlWin.left = std::stoi(tr::Cfg.get(WINDOW_LEFT));

    tr::GlWin.Cursor.x = static_cast<float>(tr::GlWin.width/2) + 0.5;
    tr::GlWin.Cursor.y = static_cast<float>(tr::GlWin.height/2) + 0.5;
    tr::GlWin.aspect = static_cast<float>(tr::GlWin.width)
                     / static_cast<float>(tr::GlWin.height);
    tr::MatProjection = glm::perspective(1.118f, tr::GlWin.aspect, 0.01f, 1000.0f);

    //  Создание 3D окна
    win_ptr = glfwCreateWindow(tr::GlWin.width, tr::GlWin.height,
                         title.c_str(), NULL, nullptr);
    // Размещение
    glfwSetWindowPos(win_ptr, tr::GlWin.left, tr::GlWin.top);

    if (nullptr == win_ptr) ERR("Creating Window fail.");
    glfwMakeContextCurrent(win_ptr);
    glfwSwapInterval(0);
    glfwSetKeyCallback(win_ptr, key_callback);
    glfwSetMouseButtonCallback(win_ptr, mouse_button_callback);
    glfwSetFramebufferSizeCallback(win_ptr, framebuffer_size_callback);
    glfwSetWindowPosCallback(win_ptr, window_pos_callback);

    if(!ogl_LoadFunctions())  ERR("Can't load OpenGl finctions");

    win_center_x = static_cast<double>(tr::GlWin.width / 2);
    win_center_y = static_cast<double>(tr::GlWin.height / 2);

    return;
  }

  //## Destructor
  window_glfw::~window_glfw()
  {
    glfwSetInputMode(win_ptr, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
    glfwTerminate();
    return;
  }

  //## Опрос положения указателя мыши и возврат его в центр окна
  // производится в конце отрисовки каждого кадра
  void window_glfw::check_mouse_pos(void)
  {
    glfwGetCursorPos(win_ptr, &mouse_x, &mouse_y);
    glfwSetCursorPos(win_ptr, win_center_x, win_center_y);

    keys.dx = static_cast<float>(mouse_x - win_center_x);
    keys.dy = static_cast<float>(mouse_y - win_center_y);
    return;
  }

  //## Show content
  void window_glfw::show(tr::scene & space)
  {
    glfwSetInputMode(win_ptr, GLFW_STICKY_KEYS, 0);

    int fps = 0;
    std::chrono::seconds one_second(1);
    std::chrono::time_point<std::chrono::system_clock> t_start, t_frame;
    std::string win_title = title + std::to_string(fps);

    t_start = std::chrono::system_clock::now();
    while (!glfwWindowShouldClose(win_ptr))
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

      glfwSwapBuffers(win_ptr);
      glfwPollEvents();
    }
    glfwDestroyWindow(win_ptr);
    return;
  }

} //namespace tr
