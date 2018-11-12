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
  double window_glfw::win_center_x = 0;
  double window_glfw::win_center_y = 0;
  //glm::vec3 ViewFrom = {};      // 3D координаты точка обзора

  // TODO: сделать привязку через конфиг
  int window_glfw::k_FRONT = GLFW_KEY_W;
  int window_glfw::k_BACK  = GLFW_KEY_S;
  int window_glfw::k_UP    = GLFW_KEY_LEFT_SHIFT;
  int window_glfw::k_DOWN  = GLFW_KEY_SPACE;
  int window_glfw::k_RIGHT = GLFW_KEY_D;
  int window_glfw::k_LEFT  = GLFW_KEY_A;

  window_gl WinGl = {};

  //## Errors callback
  void window_glfw::error_callback(int error, const char* description)
  {
    info("GLFW error " + std::to_string(error) + ": " + description);
    return;
  }

  //##
  void window_glfw::cursor_grab(GLFWwindow * window)
  {
    WinGl.show_3d(true);
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_HIDDEN);
    glfwSetCursorPos(window, win_center_x, win_center_y);
    return;
  }

  //##
  void window_glfw::cursor_free(GLFWwindow * window)
  {
    WinGl.show_3d(false);
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
    return;
  }

  //##
  void window_glfw::mouse_button_callback(
    GLFWwindow* window, int button, int action, int mods)
  {
    if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_PRESS)
      if (!WinGl.is_open) cursor_grab(window);

    keys.mouse_mods = mods;
    return;
  }

  //## Keys events callback
  void window_glfw::key_callback(GLFWwindow* window, int key, int scancode,
    int action, int mods)
  {

    keys.key_mods = mods;
    keys.key_scancode = scancode;

    if (WinGl.is_open)
    {
      keys.fb = glfwGetKey(window, k_FRONT) - glfwGetKey(window, k_BACK);
      keys.ud = glfwGetKey(window, k_DOWN)  - glfwGetKey(window, k_UP);
      keys.rl = glfwGetKey(window, k_LEFT) - glfwGetKey(window, k_RIGHT);
    }

    if (key == GLFW_KEY_ESCAPE && action == GLFW_RELEASE)
    {
      if (WinGl.is_open)
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

    return;
  }

  //## GLFW window moving callback
  void window_glfw::window_pos_callback(GLFWwindow * window,
    int left, int top)
  {
    if (!window) ERR("Error on call GLFW window_pos_callback.");
    tr::WinGl.left = static_cast<UINT>(left);
    tr::WinGl.top = static_cast<UINT>(top);
    return;
  }

  //## GLFW framebuffer callback resize
  void window_glfw::framebuffer_size_callback(GLFWwindow * window,
    int width, int height)
  {
    tr::WinGl.renew = true; // для пересчета параметров сцены

    win_center_x = static_cast<double>(width / 2);  // Координаты точки для
    win_center_y = static_cast<double>(height / 2); // установки прицела

    if (!window) ERR("Error on call GLFW framebuffer_size_callback.");
    glViewport(0, 0, width, height);

    if(width < 64)  width  = 64;
    if(height < 48) height = 48;
    tr::WinGl.width  = static_cast<UINT>(width);
    tr::WinGl.height = static_cast<UINT>(height);

    // пересчет координат курсора
    tr::WinGl.Cursor.x = static_cast<float>(tr::WinGl.width/2) + 0.5f;
    tr::WinGl.Cursor.y = static_cast<float>(tr::WinGl.height/2) + 0.5f;

    // пересчет матрицы проекции
    tr::WinGl.aspect = static_cast<float>(tr::WinGl.width)
                     / static_cast<float>(tr::WinGl.height);
    tr::MatProjection = glm::perspective(1.118f, tr::WinGl.aspect, 0.01f, 1000.0f);

    // Перестрока фреймбуфера
    glBindFramebuffer(GL_FRAMEBUFFER, Eye.frame_buf);

    // настройка размера текстуры
    glBindTexture(GL_TEXTURE_2D, Eye.texco_buf);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA,
                 static_cast<GLsizei>(tr::WinGl.width),
                 static_cast<GLsizei>(tr::WinGl.height),
                 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);

    // настройка размера рендербуфера
    glBindRenderbuffer(GL_RENDERBUFFER, Eye.rendr_buf);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8,
                 static_cast<GLsizei>(tr::WinGl.width),
                 static_cast<GLsizei>(tr::WinGl.height));

    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    return;
  }

  //## Создание нового окна с обработчиками ввода и настройка контекста
  // отображения OpenGL
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
    tr::Eye.ViewFrom.x = std::stof(tr::cfg::get(VIEW_FROM_X));
    tr::Eye.ViewFrom.y = std::stof(tr::cfg::get(VIEW_FROM_Y));
    tr::Eye.ViewFrom.z = std::stof(tr::cfg::get(VIEW_FROM_Z));
    tr::WinGl.width = static_cast<UINT>(std::stoi(tr::cfg::get(WINDOW_WIDTH)));
    tr::WinGl.height = static_cast<UINT>(std::stoi(tr::cfg::get(WINDOW_HEIGHT)));
    tr::WinGl.top = static_cast<UINT>(std::stoi(tr::cfg::get(WINDOW_TOP)));
    tr::WinGl.left = static_cast<UINT>(std::stoi(tr::cfg::get(WINDOW_LEFT)));
    tr::WinGl.Cursor.x = static_cast<float>(tr::WinGl.width/2) + 0.5f;
    tr::WinGl.Cursor.y = static_cast<float>(tr::WinGl.height/2) + 0.5f;
    tr::WinGl.aspect = static_cast<float>(tr::WinGl.width)
                     / static_cast<float>(tr::WinGl.height);
    tr::MatProjection = glm::perspective(1.118f, tr::WinGl.aspect, 0.01f, 1000.0f);

    //  Создание 3D окна
    glfwWindowHint(GLFW_VISIBLE, 0);
    win_ptr = glfwCreateWindow(static_cast<int>(tr::WinGl.width),
                               static_cast<int>(tr::WinGl.height),
                         title.c_str(), nullptr, nullptr);
    if (nullptr == win_ptr) ERR("Creating Window fail.");

    // Размещение
    glfwSetWindowPos(win_ptr, static_cast<int>(tr::WinGl.left),
                     static_cast<int>(tr::WinGl.top));
    glfwShowWindow(win_ptr);
    glfwMakeContextCurrent(win_ptr);
    glfwSwapInterval(0);
    glfwSetKeyCallback(win_ptr, key_callback);
    glfwSetMouseButtonCallback(win_ptr, mouse_button_callback);
    glfwSetCursorPosCallback(win_ptr, cursor_position_callback);
    glfwSetFramebufferSizeCallback(win_ptr, framebuffer_size_callback);
    glfwSetWindowPosCallback(win_ptr, window_pos_callback);
    if(!ogl_LoadFunctions())  ERR("Can't load OpenGl finctions");
    win_center_x = static_cast<double>(tr::WinGl.width / 2);
    win_center_y = static_cast<double>(tr::WinGl.height / 2);

    return;
  }

  //## Destructor
  window_glfw::~window_glfw()
  {
    glfwSetInputMode(win_ptr, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
    glfwTerminate();
    return;
  }

  ///
  /// Обработчик окна для перемещений курсора мыши
  /// \param ptWin - указатель окна
  /// \param xpos  - X координата курсора в окне
  /// \param ypos  - Y координата курсора в окне
  ///
  void window_glfw::cursor_position_callback(GLFWwindow* ptWin,
                                             double xpos, double ypos)
  {
    if(WinGl.is_open)
    {
      keys.dx += static_cast<float>(xpos - win_center_x);
      keys.dy += static_cast<float>(ypos - win_center_y);
      glfwSetCursorPos(ptWin, win_center_x, win_center_y);
    }
    else
    {
      // В режиме настройки перемещение указателя мыши вызывает
      // изменения вида элементов управления и перерисовку окна
      WinGl.xpos = xpos;
      WinGl.ypos = ypos;
      WinGl.renew = true;
    }
    return;
  }

  //## Show content
  void window_glfw::show(tr::scene & Scene)
  {
    glfwSetInputMode(win_ptr, GLFW_STICKY_KEYS, 0);
    int fps = 0;
    std::chrono::seconds one_second(1);
    std::chrono::time_point<std::chrono::system_clock> t_start, t_frame;
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
      Scene.draw(keys);
      if(WinGl.is_open) keys.dx = keys.dy = 0.f;
      glfwSwapBuffers(win_ptr);
      glfwPollEvents();
    }
    glfwDestroyWindow(win_ptr);
    return;
  }

} //namespace tr
