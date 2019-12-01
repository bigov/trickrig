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
// Инициализация статических членов
// ---

GLFWwindow* wglfw::win_ptr = nullptr;
GLFWwindow* wglfw::win_shared = nullptr;

std::string wglfw::title = "TrickRig: v.development";

interface_gl_context* wglfw::error_observer = nullptr;
interface_gl_context* wglfw::cursor_observer = nullptr;
interface_gl_context* wglfw::button_observer = nullptr;
interface_gl_context* wglfw::keyboard_observer = nullptr;
interface_gl_context* wglfw::position_observer = nullptr;
std::list<interface_gl_context*> wglfw::size_observers {};
interface_gl_context* wglfw::char_observer = nullptr;
interface_gl_context* wglfw::close_observer = nullptr;
interface_gl_context* wglfw::focuslost_observer = nullptr;


///
/// \brief wglfw::wglfw
///
wglfw::wglfw(GLFWwindow* SharedContext)
{
  if (!glfwInit()) ERR("Error init GLFW lib.");
  glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
  glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

#ifndef NDEBUG
  glfwWindowHint(GLFW_OPENGL_DEBUG_CONTEXT, 1);
#endif

  win_shared = SharedContext;

  //  Создание 3D окна
  glfwWindowHint(GLFW_VISIBLE, 0);
  win_ptr = glfwCreateWindow(1, 1, "", nullptr, SharedContext);
  if (nullptr == win_ptr) ERR("Creating Window fail.");
  glfwMakeContextCurrent(win_ptr);
}


///
/// Настройка окна и обработчиков ввода
///
void wglfw::set_window(u_int width, u_int height, u_int min_w, u_int min_h, u_int left, u_int top)
{
  glfwSetErrorCallback(callback_error);
  glfwSetKeyCallback(win_ptr, callback_keyboard);
  glfwSetCharCallback(win_ptr, callback_char);
  glfwSetMouseButtonCallback(win_ptr, callback_button);
  glfwSetCursorPosCallback(win_ptr, callback_cursor);
  glfwSetFramebufferSizeCallback(win_ptr, callback_size);
  glfwSetWindowPosCallback(win_ptr, callback_position);
  glfwSetWindowCloseCallback(win_ptr, callback_close);
  glfwSetWindowFocusCallback(win_ptr, callback_focus);

  glfwSetWindowSizeLimits(win_ptr, static_cast<int>(min_w), static_cast<int>(min_h),
                          GLFW_DONT_CARE, GLFW_DONT_CARE);
  glfwSetWindowSize(win_ptr, static_cast<int>(width), static_cast<int>(height));
  glfwSetWindowPos(win_ptr, static_cast<int>(left), static_cast<int>(top));
  glfwShowWindow(win_ptr);
  glfwMakeContextCurrent(win_ptr);
  glfwSwapInterval(0);  // Vertical sync is "OFF". When param is 1 - will be ON

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


void wglfw::set_error_observer(interface_gl_context& ref)
{
  error_observer = &ref;
}
void wglfw::set_cursor_observer(interface_gl_context& ref)
{
  cursor_observer = &ref;
}
void wglfw::set_button_observer(interface_gl_context& ref)
{
  button_observer = &ref;
}
void wglfw::set_keyboard_observer(interface_gl_context& ref)
{
  keyboard_observer = &ref;
}
void wglfw::set_position_observer(interface_gl_context& ref)
{
  position_observer = &ref;
}
void wglfw::add_size_observer(interface_gl_context& ref)
{
  size_observers.push_back(&ref);
}
void wglfw::set_char_observer(interface_gl_context& ref)
{
  char_observer = &ref;
}
void wglfw::set_close_observer(interface_gl_context& ref)
{
  close_observer = &ref;
}
void wglfw::set_focuslost_observer(interface_gl_context& ref)
{
  focuslost_observer = &ref;
}


///
/// \brief wglfw::swap_buffers
///
void wglfw::swap_buffers(void)
{
  glfwSwapBuffers(win_ptr);
  glfwPollEvents();
}


///
/// \brief wglfw::cursor_hide
///
void wglfw::cursor_hide(void)
{
  glfwSetInputMode(win_ptr, GLFW_CURSOR, GLFW_CURSOR_HIDDEN);

  // отключить ввод символов
  glfwSetCharCallback(win_ptr, nullptr);
}


///
/// \brief wglfw::cursor_restore
///
void wglfw::cursor_restore(void)
{
  glfwSetInputMode(win_ptr, GLFW_CURSOR, GLFW_CURSOR_NORMAL);

  // разрешить ввод символов
  glfwSetCharCallback(win_ptr, callback_char);
}


///
/// \brief wglfw::set_cursor_pos
/// \param x
/// \param y
///
void wglfw::set_cursor_pos(double x, double y)
{
  glfwSetCursorPos(win_ptr, x, y);
}


///
/// \brief wglfw::get_frame_buffer_size
/// \param width
/// \param height
///
void wglfw::get_frame_buffer_size(int* width, int* height)
{
  glfwGetFramebufferSize(win_ptr, width, height);
}


///
/// \brief wglfw::get_win_id
/// \return
///
GLFWwindow* wglfw::get_win_id(void) const
{
  return win_ptr;
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
void wglfw::callback_error(int error, const char* description)
{
  std::string Message = "GLFW error " + std::to_string(error) + ": " + description;
  if(error_observer != nullptr) error_observer->error_event(Message.c_str());
}


///
/// Обработчик перемещений курсора мыши
/// \param ptWin - указатель окна
/// \param xpos  - X координата курсора в окне
/// \param ypos  - Y координата курсора в окне
///
void wglfw::callback_cursor(GLFWwindow* w, double x, double y)
{
  assert(w == win_ptr);
  if(cursor_observer != nullptr) cursor_observer->cursor_event(x, y);
}


///
/// \brief window_glfw::mouse_button_callback
/// \param window
/// \param button
/// \param action
/// \param mods
///
void wglfw::callback_button(GLFWwindow* w, int button, int action, int mods)
{
  assert(w == win_ptr);
  if(button_observer != nullptr) button_observer->mouse_event(button, action, mods);
}


///
/// Keys events callback
///
void wglfw::callback_keyboard(GLFWwindow* w, int key, int scancode, int action, int mods)
{
  assert(w == win_ptr);
  if(keyboard_observer != nullptr) keyboard_observer->keyboard_event(key, scancode, action, mods);
}


///
/// GLFW window moving callback
///
void wglfw::callback_position(GLFWwindow* w, int left, int top)
{
  assert(w == win_ptr);
  if(position_observer != nullptr) position_observer->reposition_event(left, top);
}


///
/// \brief wglfw::resize_callback
/// \param WindowPointer
/// \param width
/// \param height
/// \details GLFW framebuffer and window-data callback resize
///
void wglfw::callback_size(GLFWwindow* w, int width, int height)
{
  assert(w == win_ptr);
  for(auto& observer: size_observers) observer->resize_event(width, height);
}


///
/// \brief glfw_wr::character_callback
/// \param window
/// \param key
///
void wglfw::callback_char(GLFWwindow* w, u_int ch)
{
  assert(w == win_ptr);
  if(char_observer != nullptr) char_observer->character_event(ch);
}


///
/// \brief wglfw::window_close_callback
/// \param w
///
void wglfw::callback_close(GLFWwindow* w)
{
  assert(w == win_ptr);
  if(close_observer != nullptr) close_observer->close_event();
}


///
/// \brief wglfw::window_focus_callback
/// \param w
///
void wglfw::callback_focus(GLFWwindow* w,  int focused)
{
  assert(w == win_ptr);
  if(0 == focused)
  {
    if(focuslost_observer != nullptr) focuslost_observer->focus_lost_event();
  }
}

} //namespace tr
