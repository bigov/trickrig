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
  glViewport(0, 0, GLsizei(w), GLsizei(h)); // пересчет Viewport

  // пересчет позции координат прицела (центр окна)
  Cursor.x = static_cast<float>(w/2);
  Cursor.y = static_cast<float>(h/2);

  // пересчет матрицы проекции
  aspect = static_cast<float>(w) / static_cast<float>(h);
  MatProjection = glm::perspective(1.118f, aspect, zNear, zFar);

  // пересчет рендер-буфера
  if(nullptr != RenderBuffer) RenderBuffer->resize(GLsizei(w), GLsizei(h));
  if(nullptr != pWinGui) pWinGui->resize(w, h);
}


///
/// Создание нового окна с обработчиками ввода и настройка контекста
/// отображения OpenGL
///
wglfw::wglfw(void)
{
  glfwSetErrorCallback(error_callback);
  if (!glfwInit()) ERR("Error init GLFW lib.");

  glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
  glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
  //glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, 0);

  #ifndef NDEBUG
  glfwWindowHint(GLFW_OPENGL_DEBUG_CONTEXT, 1);
  #endif

  //  Создание 3D окна
  glfwWindowHint(GLFW_VISIBLE, 0);
  win_ptr = glfwCreateWindow(static_cast<int>(AppWindow.width),
                             static_cast<int>(AppWindow.height),
                       title.c_str(), nullptr, nullptr);
  if (nullptr == win_ptr) ERR("Creating Window fail.");

  glfwSetWindowSizeLimits(win_ptr, GLsizei(AppWindow.minwidth), GLsizei(AppWindow.minheight),
                          GLFW_DONT_CARE, GLFW_DONT_CARE);

  glfwSetWindowPos(win_ptr, static_cast<int>(tr::AppWindow.left),
                   static_cast<int>(tr::AppWindow.top));

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

  glfwSetInputMode(win_ptr, GLFW_STICKY_KEYS, 0);
}


///
///  Destructor
///
wglfw::~wglfw()
{
  cursor_restore();
  if(!glfwWindowShouldClose(win_ptr)) glfwSetWindowShouldClose(win_ptr, true);
  glfwDestroyWindow(win_ptr);
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
/// \brief wglfw::cursor_hide
///
void wglfw::cursor_hide(void)
{
  // спрятать указатель мыши
  glfwSetInputMode(win_ptr, GLFW_CURSOR, GLFW_CURSOR_HIDDEN);
  AppWindow.xpos = AppWindow.width/2;
  AppWindow.ypos = AppWindow.height/2;
  glfwSetCursorPos(win_ptr, AppWindow.xpos, AppWindow.ypos);
}


///
/// \brief wglfw::cursor_restore
///
void wglfw::cursor_restore(void)
{
  // восстановить курсор мыши
  glfwSetInputMode(win_ptr, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
  Input.fb = 0; Input.ud = 0; Input.rl = 0;
  Input.dx = 0; Input.dy = 0;
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
  Input.mods   = mods;
  Input.mouse  = button;
  Input.action = action;
}


///
/// Keys events callback
///
void wglfw::key_callback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
  Input.key    = key;
  Input.action = action;
  Input.mouse  = -1;
  Input.mods = mods;
  Input.scancode = scancode;
  Input.fb = glfwGetKey(window, k_FRONT) - glfwGetKey(window, k_BACK);
  Input.ud = glfwGetKey(window, k_DOWN)  - glfwGetKey(window, k_UP);
  Input.rl = glfwGetKey(window, k_LEFT)  - glfwGetKey(window, k_RIGHT);
}


///
/// \brief glfw_wr::character_callback
/// \param window
/// \param key
///
void wglfw::character_callback(GLFWwindow*, u_int ch)
{
  if(!Input.text_mode) return;

  if(ch < 128)
  {
    Input.StringBuffer += char(ch);
  }
  else
  {
    auto str = wstring2string({static_cast<wchar_t>(ch)});
    if(str == "№") str = "N";     // № трехбайтный, поэтому заменим на N
    if(str.size() > 2) str = "_"; // блокировка 3-х байтных символов
    Input.StringBuffer += str;
  }
}


///
/// GLFW window moving callback
///
void wglfw::window_pos_callback(GLFWwindow*, int left, int top)
{
  AppWindow.left = static_cast<u_int>(left);
  AppWindow.top = static_cast<u_int>(top);
}


///
/// GLFW framebuffer callback resize
///
void wglfw::framebuffer_size_callback(GLFWwindow*, int width, int height)
{
  AppWindow.resize(static_cast<u_int>(width), static_cast<u_int>(height));
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
    Input.dx += static_cast<float>(x - AppWindow.xpos);
    Input.dy += static_cast<float>(y - AppWindow.ypos);
    glfwSetCursorPos(ptWin, AppWindow.xpos, AppWindow.ypos);
  }
  else
  {
    AppWindow.xpos = x;
    AppWindow.ypos = y;
  }
}


///
/// \brief wglfw::swap_buffers
///
void wglfw::swap_buffers(void)
{
  glfwSwapBuffers(win_ptr);
  glfwPollEvents();
  AppWindow.is_open &= !glfwWindowShouldClose(win_ptr);
}

} //namespace tr
