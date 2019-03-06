//============================================================================
//
// file wglfw.cpp
//
// интерфейс к библиотеке GLFW
//
//============================================================================
#include "wglfw.hpp"

namespace tr
{
evInput wglfw::keys = {0.0, 0.0, 0, 0, 0, -1, -1, -1, -1, -1};
std::string wglfw::title = "TrickRig: v.development";

// TODO: сделать привязку через конфиг
int wglfw::k_FRONT = GLFW_KEY_W;
int wglfw::k_BACK  = GLFW_KEY_S;
int wglfw::k_UP    = GLFW_KEY_LEFT_SHIFT;
int wglfw::k_DOWN  = GLFW_KEY_SPACE;
int wglfw::k_RIGHT = GLFW_KEY_D;
int wglfw::k_LEFT  = GLFW_KEY_A;

const int MOUSE_BUTTON_LEFT  = GLFW_MOUSE_BUTTON_LEFT;
const int MOUSE_BUTTON_RIGHT = GLFW_MOUSE_BUTTON_RIGHT;
const int PRESS              = GLFW_PRESS;
const int REPEAT             = GLFW_REPEAT;
const int RELEASE            = GLFW_RELEASE;
const int KEY_ESCAPE         = GLFW_KEY_ESCAPE;
const int KEY_BACKSPACE      = GLFW_KEY_BACKSPACE;


///
/// \brief main_window::resize
/// \param w
/// \param h
///
void main_window::resize(u_int w, u_int h)
{
  width  = w;
  height = h;
  glViewport(0, 0, w, h); // пересчет Viewport

  // пересчет позции координат прицела (центр окна)
  Cursor.x = static_cast<float>(w/2) + 0.5f;
  Cursor.y = static_cast<float>(h/2) + 0.5f;

  // пересчет матрицы проекции
  aspect = static_cast<float>(w) / static_cast<float>(h);
  MatProjection = glm::perspective(1.118f, aspect, 0.01f, 1000.0f);

  if(nullptr != pFrBuffer ) pFrBuffer->resize(w, h); // пересчет рендер-буфера
  if(nullptr != pWinGui) pWinGui->resize(w, h);
}


///
/// Создание нового окна с обработчиками ввода и настройка контекста
/// отображения OpenGL
///
wglfw::wglfw(void)
{
  cfg::load_app_cfg();

  glfwSetErrorCallback(error_callback);
  if (!glfwInit()) ERR("Error init GLFW lib.");

  glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
  //glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_COMPAT_PROFILE);
  //glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, 0);

  #ifndef NDEBUG
  glfwWindowHint(GLFW_OPENGL_DEBUG_CONTEXT, 1);
  #endif

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
  glfwSwapInterval(0);  // Vertical sync is "OFF". When param is 1 - will be ON
  glfwSetKeyCallback(win_ptr, key_callback);
  glfwSetCharCallback(win_ptr, character_callback);
  glfwSetMouseButtonCallback(win_ptr, mouse_button_callback);
  glfwSetCursorPosCallback(win_ptr, cursor_position_callback);
  glfwSetFramebufferSizeCallback(win_ptr, framebuffer_size_callback);
  glfwSetWindowPosCallback(win_ptr, window_pos_callback);

  if(!gladLoadGLLoader(GLADloadproc(glfwGetProcAddress)))
  if(!gladLoadGL()) { ERR("FAILURE: can't load GLAD."); }

  Scene = std::make_unique<tr::scene>();

}


///
///  Destructor
///
wglfw::~wglfw()
{
  Scene = nullptr; // destruct Scene
  glfwSetInputMode(win_ptr, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
  glfwTerminate();
}


///
/// Errors callback
///
void wglfw::error_callback(int error, const char* description)
{
  info("GLFW error " + std::to_string(error) + ": " + description);
}


///
/// \brief wingl::set_cursor Смена режима отображения сцены (GUI/3D)
///
void wglfw::set_cursor(void)
{
  if(AppWin.set_mouse_ptr < 0 )
  {
    // спрятать указатель мыши
    glfwSetInputMode(win_ptr, GLFW_CURSOR, GLFW_CURSOR_HIDDEN);
    AppWin.xpos = AppWin.width/2;
    AppWin.ypos = AppWin.height/2;
    glfwSetCursorPos(win_ptr, AppWin.xpos, AppWin.ypos);
  }
  else
  {
    // восстановить курсор мыши
    glfwSetInputMode(win_ptr, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
    keys.fb = 0; keys.ud = 0; keys.rl = 0;
    keys.dx = 0; keys.dy = 0;
  }

  AppWin.set_mouse_ptr = 0; // после обработки установить нейтральное значение
}


///
/// \brief window_glfw::mouse_button_callback
/// \param window
/// \param button
/// \param action
/// \param mods
///
void wglfw::mouse_button_callback(GLFWwindow*, int button, int action, int mods)
{
  keys.mods   = mods;
  keys.mouse  = button;
  keys.action = action;
}


///
/// Keys events callback
///
void wglfw::key_callback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
  keys.key    = key;
  keys.action = action;
  keys.mouse  = -1;
  keys.mods = mods;
  keys.scancode = scancode;
  keys.fb = glfwGetKey(window, k_FRONT) - glfwGetKey(window, k_BACK);
  keys.ud = glfwGetKey(window, k_DOWN)  - glfwGetKey(window, k_UP);
  keys.rl = glfwGetKey(window, k_LEFT)  - glfwGetKey(window, k_RIGHT);
}


///
/// \brief glfw_wr::character_callback
/// \param window
/// \param key
///
void wglfw::character_callback(GLFWwindow*, u_int ch)
{
  if(AppWin.pInputBuffer != nullptr)
  {
    if(ch < 128)
    {
      *(AppWin.pInputBuffer) += ch;
    }
    else
    {
      auto str = wstring2string({static_cast<wchar_t>(ch)});
      if(str == u8"№") str = "N";     // № трехбайтный, поэтому заменим на N
      if(str.size() > 2) str = "_";   // блокировка 3-х байтных символов
      *(AppWin.pInputBuffer) += str;
    }
  }
}


///
/// GLFW window moving callback
///
void wglfw::window_pos_callback(GLFWwindow*, int left, int top)
{
  AppWin.left = static_cast<u_int>(left);
  AppWin.top = static_cast<u_int>(top);
}


///
/// GLFW framebuffer callback resize
///
void wglfw::framebuffer_size_callback(GLFWwindow*, int width, int height)
{
  AppWin.resize(static_cast<u_int>(width), static_cast<u_int>(height));
}


///
/// Обработчик окна для перемещений курсора мыши
/// \param ptWin - указатель окна
/// \param xpos  - X координата курсора в окне
/// \param ypos  - Y координата курсора в окне
///
void wglfw::cursor_position_callback(GLFWwindow* ptWin, double x, double y)
{
  if(glfwGetInputMode(ptWin, GLFW_CURSOR) == GLFW_CURSOR_HIDDEN)
  {
    keys.dx += static_cast<float>(x - AppWin.xpos);
    keys.dy += static_cast<float>(y - AppWin.ypos);
    glfwSetCursorPos(ptWin, AppWin.xpos, AppWin.ypos);
  }
  else
  {
    AppWin.xpos = x;
    AppWin.ypos = y;
  }
}


///
/// \brief Main loop for the app-window show
///
void wglfw::show(void)
{
  glfwSetInputMode(win_ptr, GLFW_STICKY_KEYS, 0);
  int fps = 0;
  std::chrono::seconds one_second(1);
  std::chrono::time_point<std::chrono::system_clock> t_start, t_frame;
  t_start = std::chrono::system_clock::now();

  while (AppWin.run && (!glfwWindowShouldClose(win_ptr)))
  {
    fps++;
    t_frame = std::chrono::system_clock::now();
    if (t_frame - t_start >= one_second)
    {
      t_start = std::chrono::system_clock::now();
      AppWin.fps = fps;
      fps = 0;
    }
    Scene->draw(keys);
    if(AppWin.set_mouse_ptr != 0) set_cursor();
    glfwSwapBuffers(win_ptr);
    glfwPollEvents();
  }

  if(!glfwWindowShouldClose(win_ptr)) glfwSetWindowShouldClose(win_ptr, true);
  glfwDestroyWindow(win_ptr);
}

} //namespace tr
