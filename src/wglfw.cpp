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


// Инициализация статических членов
IWindowInput* wglfw::win_observer = nullptr;
IWindowInput* wglfw::char_observer = nullptr;
IWindowInput* wglfw::size_observer = nullptr;

GLFWwindow* wglfw::win_ptr = nullptr;
double wglfw::half_w = 0.0;   // середина окна по X
double wglfw::half_h = 0.0;   // середина окна по Y

///
/// Создание нового окна с обработчиками ввода и настройка контекста
/// отображения OpenGL
///
wglfw::wglfw(int width, int height, int min_w, int min_h, int left, int top)
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
  win_ptr = glfwCreateWindow(width, height, title.c_str(), nullptr, nullptr);
  if (nullptr == win_ptr) ERR("Creating Window fail.");
  half_w = width / 2.0;
  half_h = height / 2.0;

  glfwSetWindowSizeLimits(win_ptr, min_w, min_h, GLFW_DONT_CARE, GLFW_DONT_CARE);
  glfwSetWindowPos(win_ptr, left, top);
  glfwShowWindow(win_ptr);
  glfwMakeContextCurrent(win_ptr);
  glfwSwapInterval(0);  // Vertical sync is "OFF". When param is 1 - will be ON
  glfwSetKeyCallback(win_ptr, keyboard_callback);
  glfwSetCharCallback(win_ptr, character_callback);
  glfwSetMouseButtonCallback(win_ptr, mouse_button_callback);
  glfwSetCursorPosCallback(win_ptr, cursor_position_callback);
  glfwSetFramebufferSizeCallback(win_ptr, resize_callback);
  glfwSetWindowPosCallback(win_ptr, reposition_callback);
  glfwSetWindowCloseCallback(win_ptr, window_close_callback);

  if(!gladLoadGLLoader(GLADloadproc(glfwGetProcAddress)))
  if(!gladLoadGL()) { ERR("FAILURE: can't load GLAD."); }

  glfwSetInputMode(win_ptr, GLFW_STICKY_KEYS, 0);
}


///
///  Destructor
///
wglfw::~wglfw()
{
  if(!glfwWindowShouldClose(win_ptr)) glfwSetWindowShouldClose(win_ptr, true);
  glfwDestroyWindow(win_ptr);
  glfwTerminate();
}


///
/// \brief wglfw::set_win_observer
/// \param ref
/// \details добавить наблюдателя за событиями окна
///
void wglfw::set_win_observer(IWindowInput& ref)
{
  win_observer = &ref;
}


///
/// \brief wglfw::set_char_observer
/// \param ref
/// \details добавить наблюдателя за событиями ввода
///
void wglfw::set_char_observer(IWindowInput& ref)
{
  char_observer = &ref;
}


///
/// \brief wglfw::set_size_observer
/// \param ref
/// \details добавить наблюдателя за событиями изменения размеров
///
void wglfw::set_size_observer(IWindowInput& ref)
{
  size_observer = &ref;
}


///
/// \brief wglfw::error_callback
/// \param error
/// \param description
///
/// \details Errors callback
///
/// \todo Если тут настроить обработчик через вызов метода
/// "наблюдателя", то можно полностью сделать класс wglfw
/// независимым, свободным от внешних связей модулем.
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
  glfwSetCursorPosCallback(win_ptr, sight_position_callback);
  glfwSetCursorPos(win_ptr, half_w, half_h); // курсор в центр окна
  glfwSetCharCallback(win_ptr, nullptr);

  if(win_observer != nullptr) win_observer->cursor_hide();
}


///
/// \brief wglfw::cursor_restore
///
void wglfw::cursor_restore(void)
{
  // восстановить курсор мыши
  glfwSetInputMode(win_ptr, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
  glfwSetCursorPosCallback(win_ptr, cursor_position_callback);
  glfwSetCursorPos(win_ptr, half_w, half_h); // Установить курсор в центре окна
  glfwSetCharCallback(win_ptr, character_callback);

  if(win_observer != nullptr) win_observer->cursor_show();
}


///
/// \brief window_glfw::mouse_button_callback
/// \param window
/// \param button
/// \param action
/// \param mods
///
void wglfw::mouse_button_callback(GLFWwindow* w, int button, int action, int mods)
{
  assert(w == win_ptr);
  if(win_observer != nullptr) win_observer->mouse_event(button, action, mods);
}


///
/// Keys events callback
///
void wglfw::keyboard_callback(GLFWwindow* w, int key, int scancode, int action, int mods)
{
  assert(w == win_ptr);
  if(win_observer != nullptr) win_observer->keyboard_event(key, scancode, action, mods);
}


///
/// \brief glfw_wr::character_callback
/// \param window
/// \param key
///
void wglfw::character_callback(GLFWwindow* w, u_int ch)
{
  assert(w == win_ptr);
  if(char_observer != nullptr) char_observer->character_event(ch);
}


///
/// GLFW window moving callback
///
void wglfw::reposition_callback(GLFWwindow* w, int left, int top)
{
  assert(w == win_ptr);
  if(win_observer != nullptr) win_observer->reposition_event(left, top);
}


///
/// \brief wglfw::resize_callback
/// \param WindowPointer
/// \param width
/// \param height
/// \details GLFW framebuffer and window-data callback resize
///
void wglfw::resize_callback(GLFWwindow* w, int width, int height)
{
  assert(w == win_ptr);
  glViewport(0, 0, width, height); // пересчет Viewport
  half_w = width / 2.0;            // пересчет центра окна
  half_h = height / 2.0;

  if(win_observer != nullptr) win_observer->resize_event(width, height);
  if(size_observer != nullptr) size_observer->resize_event(width, height);
}


///
/// Обработчик перемещений курсора мыши
/// \param ptWin - указатель окна
/// \param xpos  - X координата курсора в окне
/// \param ypos  - Y координата курсора в окне
///
void wglfw::cursor_position_callback(GLFWwindow* w, double x, double y)
{
  assert(w == win_ptr);
  if(win_observer != nullptr) win_observer->cursor_position_event(x, y);
}


///
/// Обработчик смещения прицела в 3D режиме
/// \param ptWin - указатель окна
/// \param xpos  - X координата курсора в окне
/// \param ypos  - Y координата курсора в окне
///
void wglfw::sight_position_callback(GLFWwindow* w, double x, double y)
{
  assert(w == win_ptr);
  if(win_observer != nullptr) win_observer->sight_position_event(x, y);
  glfwSetCursorPos(w, half_w, half_h);
}


///
/// \brief wglfw::window_close_callback
/// \param w
///
void wglfw::window_close_callback(GLFWwindow* w)
{
  assert(w == win_ptr);
  if(win_observer != nullptr) win_observer->close_event();
}


///
/// \brief wglfw::swap_buffers
///
void wglfw::swap_buffers(void)
{
  glfwSwapBuffers(win_ptr);
  glfwPollEvents();
}

} //namespace tr
