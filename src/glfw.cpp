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
  evInput glfw_wr::keys = {0.0, 0.0, 0, 0, 0, 0, 0, 0};
  std::string glfw_wr::title = "TrickRig: v.development";
  //int glfw_wr::win_center_x = 0;
  //int glfw_wr::win_center_y = 0;
  //glm::vec3 ViewFrom = {};      // 3D координаты точка обзора

  // TODO: сделать привязку через конфиг
  int glfw_wr::k_FRONT = GLFW_KEY_W;
  int glfw_wr::k_BACK  = GLFW_KEY_S;
  int glfw_wr::k_UP    = GLFW_KEY_LEFT_SHIFT;
  int glfw_wr::k_DOWN  = GLFW_KEY_SPACE;
  int glfw_wr::k_RIGHT = GLFW_KEY_D;
  int glfw_wr::k_LEFT  = GLFW_KEY_A;

  main_window AppWin = {};

  ///
  /// Errors callback
  ///
  void glfw_wr::error_callback(int error, const char* description)
  {
    info("GLFW error " + std::to_string(error) + ": " + description);
    return;
  }

  ///
  /// \brief window_glfw::scene_open
  /// \param window
  ///
  void glfw_wr::scene_open(GLFWwindow * window)
  {
    AppWin.set_mode(COVER_OFF);
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_HIDDEN);
    AppWin.xpos = AppWin.width/2;
    AppWin.ypos = AppWin.height/2;
    glfwSetCursorPos(window, AppWin.xpos, AppWin.ypos);
    AppWin.newsize = true; // для обновления текстуры фреймбуфера
    return;
  }

  ///
  /// \brief window_glfw::cursor_free
  /// \param window
  ///
  void glfw_wr::cursor_free(GLFWwindow * window)
  {
    AppWin.set_mode(COVER_LOCATION);
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
    return;
  }

  ///
  /// \brief window_glfw::mouse_button_callback
  /// \param window
  /// \param button
  /// \param action
  /// \param mods
  ///
  void glfw_wr::mouse_button_callback(
    GLFWwindow* window, int button, int action, int mods)
  {
    if( AppWin.cover != COVER_OFF)
    {
      AppWin.mouse_lbutton_on = (button == GLFW_MOUSE_BUTTON_LEFT &&
                                action == GLFW_PRESS);

      if(button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_RELEASE )
      {
        switch (AppWin.OverButton) {
          case BTN_OPEN:
            scene_open(window);
            break;
          case BTN_CONFIG:
            AppWin.cover = COVER_CONFIG;
            break;
          case BTN_LOCATION:
            AppWin.cover = COVER_LOCATION;
            break;
          case BTN_CREATE:
            AppWin.user_input.clear();
            AppWin.cover = COVER_CREATE;
            break;
          case BTN_ENTER_NAME:
            AppWin.cover = COVER_LOCATION;
            break;
          case BTN_CLOSE:
            glfwSetWindowShouldClose(window, true);
            break;
          case NONE:
            break;
        }
      }
    }

    keys.mouse_mods = mods;
    return;
  }

  ///
  /// \brief glfw_wr::escape_key
  /// \param window
  ///
  void glfw_wr::escape_key(GLFWwindow* window)
  {
    switch (AppWin.cover) {
      case COVER_OFF:
        keys.fb = 0;
        keys.ud = 0;
        keys.rl = 0;
        keys.dx = 0;
        keys.dy = 0;
        cursor_free(window);
        break;
      case COVER_LOCATION:
        AppWin.cover = COVER_START;
        break;
      case COVER_CREATE:
        AppWin.cover = COVER_LOCATION;
        break;
      case COVER_CONFIG:
        AppWin.cover = COVER_START;
        break;
      case COVER_START:
        glfwSetWindowShouldClose(window, true);
    }
    return;
  }

  ///
  /// Keys events callback
  ///
  void glfw_wr::key_callback(GLFWwindow* window, int key, int scancode,
    int action, int mods)
  {

    keys.key_mods = mods;
    keys.key_scancode = scancode;

    if (AppWin.cover == COVER_OFF)
    {
      keys.fb = glfwGetKey(window, k_FRONT) - glfwGetKey(window, k_BACK);
      keys.ud = glfwGetKey(window, k_DOWN)  - glfwGetKey(window, k_UP);
      keys.rl = glfwGetKey(window, k_LEFT) - glfwGetKey(window, k_RIGHT);
    }
    else
    {
      if (key == GLFW_KEY_BACKSPACE && action == GLFW_PRESS)
        AppWin.key_backspace = true;
    }

    if (key == GLFW_KEY_ESCAPE && action == GLFW_RELEASE)
      escape_key(window);

    return;
  }

  ///
  /// \brief glfw_wr::character_callback
  /// \param window
  /// \param key
  ///
  void glfw_wr::character_callback(GLFWwindow*, u_int ch)
  {
    if(COVER_CREATE == AppWin.cover)
    {
      AppWin.user_input += static_cast<wchar_t>(ch);
    }
    return;
  }

  ///
  /// GLFW window moving callback
  ///
  void glfw_wr::window_pos_callback(GLFWwindow*,
    int left, int top)
  {
    tr::AppWin.left = static_cast<u_int>(left);
    tr::AppWin.top = static_cast<u_int>(top);
    return;
  }

  ///
  /// GLFW framebuffer callback resize
  ///
  void glfw_wr::framebuffer_size_callback(GLFWwindow*,
    int width, int height)
  {
    AppWin.width  = static_cast<u_int>(width);
    AppWin.height = static_cast<u_int>(height);
    AppWin.newsize = true; // для пересчета фреймбуфера
    return;
  }

  ///
  /// Создание нового окна с обработчиками ввода и настройка контекста
  /// отображения OpenGL
  ///
  glfw_wr::glfw_wr(void)
  {
    glfwSetErrorCallback(error_callback);
    if (!glfwInit()) ERR("Error init GLFW lib.");

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 2);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, 0);

    #ifndef NDEBUG
    glfwWindowHint(GLFW_OPENGL_DEBUG_CONTEXT, 1);
    #endif

    // Начальные настройки камеры вида и размера окна
    Eye.ViewFrom.x = std::stof(cfg::get(VIEW_FROM_X));
    Eye.ViewFrom.y = std::stof(cfg::get(VIEW_FROM_Y));
    Eye.ViewFrom.z = std::stof(cfg::get(VIEW_FROM_Z));

    Eye.look_a = std::stof(tr::cfg::get(LOOK_AZIM));
    Eye.look_t = std::stof(tr::cfg::get(LOOK_TANG));

    AppWin.width = static_cast<u_int>(std::stoi(cfg::get(WINDOW_WIDTH)));
    AppWin.height = static_cast<u_int>(std::stoi(cfg::get(WINDOW_HEIGHT)));
    AppWin.top = static_cast<u_int>(std::stoi(cfg::get(WINDOW_TOP)));
    AppWin.left = static_cast<u_int>(std::stoi(cfg::get(WINDOW_LEFT)));
    AppWin.Cursor.x = static_cast<float>(AppWin.width/2) + 0.5f;
    AppWin.Cursor.y = static_cast<float>(AppWin.height/2) + 0.5f;
    AppWin.aspect = static_cast<float>(AppWin.width)
                     / static_cast<float>(AppWin.height);

    //  Создание 3D окна
    glfwWindowHint(GLFW_VISIBLE, 0);
    win_ptr = glfwCreateWindow(static_cast<int>(AppWin.width),
                               static_cast<int>(AppWin.height),
                         title.c_str(), nullptr, nullptr);
    if (nullptr == win_ptr) ERR("Creating Window fail.");

    glfwSetWindowSizeLimits(win_ptr, AppWin.minwidth, AppWin.minheight,
                            GLFW_DONT_CARE, GLFW_DONT_CARE);

    glfwSetWindowPos(win_ptr, static_cast<int>(tr::AppWin.left),
                     static_cast<int>(tr::AppWin.top));

    glfwShowWindow(win_ptr);
    glfwMakeContextCurrent(win_ptr);
    glfwSwapInterval(0);
    glfwSetKeyCallback(win_ptr, key_callback);
    glfwSetCharCallback(win_ptr, character_callback);
    glfwSetMouseButtonCallback(win_ptr, mouse_button_callback);
    glfwSetCursorPosCallback(win_ptr, cursor_position_callback);
    glfwSetFramebufferSizeCallback(win_ptr, framebuffer_size_callback);
    glfwSetWindowPosCallback(win_ptr, window_pos_callback);
    if(!ogl_LoadFunctions()) ERR("Can't load OpenGl finctions");

    return;
  }

  ///
  ///  Destructor
  ///
  glfw_wr::~glfw_wr()
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
  void glfw_wr::cursor_position_callback(GLFWwindow* ptWin,
                                             double x, double y)
  {
    if(AppWin.cover == COVER_OFF)
    {
      keys.dx += static_cast<float>(x - AppWin.xpos);
      keys.dy += static_cast<float>(y - AppWin.ypos);
      glfwSetCursorPos(ptWin, AppWin.xpos, AppWin.ypos);
    }
    else
    {
      // В режиме настройки перемещение указателя мыши вызывает
      // изменения вида элементов управления и перерисовку окна
      AppWin.xpos = x;
      AppWin.ypos = y;
    }
    return;
  }

  ///
  /// Show content
  ///
  void glfw_wr::show(tr::scene & Scene)
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
        AppWin.fps = fps;
        fps = 0;
      }
      Scene.draw(keys);
      if(AppWin.cover == COVER_OFF) keys.dx = keys.dy = 0.f;
      glfwSwapBuffers(win_ptr);
      glfwPollEvents();
    }
    glfwDestroyWindow(win_ptr);
    return;
  }

} //namespace tr
