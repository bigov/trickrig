//============================================================================
//
// file trgl.cpp
//
// интерфейс к библиотеке GLFW
//
//============================================================================
#include "trgl.hpp"

namespace tr
{
// Инициализация статических членов
// --- --- ---
interface_gl_context* trgl::error_observer = nullptr;
interface_gl_context* trgl::cursor_observer = nullptr;
interface_gl_context* trgl::button_observer = nullptr;
interface_gl_context* trgl::keyboard_observer = nullptr;
interface_gl_context* trgl::position_observer = nullptr;
std::list<interface_gl_context*> trgl::size_observers {};
interface_gl_context* trgl::char_observer = nullptr;
interface_gl_context* trgl::close_observer = nullptr;
interface_gl_context* trgl::focuslost_observer = nullptr;


///
/// \brief wglfw::wglfw
///
trgl::trgl(const char* title)
{
  if (!glfwInit()) ERR("Fatal error: can't init GLFW lib.");
  glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
  glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
  glfwWindowHint(GLFW_VISIBLE, 0);

#ifndef NDEBUG
  glfwWindowHint(GLFW_OPENGL_DEBUG_CONTEXT, 1);
#endif

  win_main = glfwCreateWindow(1, 1, title, nullptr, nullptr);
  if (nullptr == win_main) ERR("Creating Window fail.");
  win_thread = glfwCreateWindow(1, 1, "", nullptr, win_main);
  if (nullptr == win_thread) ERR("Creating sub-window fail.");
  glfwMakeContextCurrent(win_main);

  if(!gladLoadGLLoader(GLADloadproc(glfwGetProcAddress)))
  {
    if(!gladLoadGL()) ERR("Critical error: can't load GLAD.");
  }
}


///
/// Настройка окна и обработчиков ввода
///
void trgl::set_window(uint width, uint height, uint min_w, uint min_h, uint left, uint top)
{
  glfwSetErrorCallback(callback_error);
  glfwSetKeyCallback(win_main, callback_keyboard);
  glfwSetCharCallback(win_main, callback_char);
  glfwSetMouseButtonCallback(win_main, callback_button);
  glfwSetCursorPosCallback(win_main, callback_cursor);
  glfwSetFramebufferSizeCallback(win_main, callback_size);
  glfwSetWindowPosCallback(win_main, callback_position);
  glfwSetWindowCloseCallback(win_main, callback_close);
  glfwSetWindowFocusCallback(win_main, callback_focus);

  glfwSetWindowSizeLimits(win_main, static_cast<int>(min_w), static_cast<int>(min_h),
                          GLFW_DONT_CARE, GLFW_DONT_CARE);
  glfwSetWindowSize(win_main, static_cast<int>(width), static_cast<int>(height));
  glfwSetWindowPos(win_main, static_cast<int>(left), static_cast<int>(top));
  glfwShowWindow(win_main);
  glfwSwapInterval(0);  // Vertical sync is "OFF". When param is 1 - will be ON
  glfwSetInputMode(win_main, GLFW_STICKY_KEYS, 0);
}


///
///  Destructor
///
trgl::~trgl(void)
{
  if(nullptr != win_thread) glfwDestroyWindow(win_thread);
  win_thread = nullptr;
  if(nullptr != win_main) glfwDestroyWindow(win_main);
  win_main = nullptr;
  glfwTerminate();
}


void trgl::set_error_observer(interface_gl_context& ref)
{
  error_observer = &ref;
}
void trgl::set_cursor_observer(interface_gl_context& ref)
{
  cursor_observer = &ref;
}
void trgl::set_mbutton_observer(interface_gl_context& ref)
{
  button_observer = &ref;
}
void trgl::set_keyboard_observer(interface_gl_context& ref)
{
  keyboard_observer = &ref;
}
void trgl::set_position_observer(interface_gl_context& ref)
{
  position_observer = &ref;
}
void trgl::add_size_observer(interface_gl_context& ref)
{
  size_observers.push_back(&ref);
}
void trgl::set_char_observer(interface_gl_context& ref)
{
  char_observer = &ref;
}
void trgl::set_close_observer(interface_gl_context& ref)
{
  close_observer = &ref;
}
void trgl::set_focuslost_observer(interface_gl_context& ref)
{
  focuslost_observer = &ref;
}


///
/// \brief wglfw::swap_buffers
///
void trgl::swap_buffers(void)
{
  glfwSwapBuffers(win_main);
  glfwPollEvents();
}


///
/// \brief wglfw::cursor_hide
///
void trgl::cursor_hide(void)
{
  glfwSetInputMode(win_main, GLFW_CURSOR, GLFW_CURSOR_HIDDEN);

  // отключить ввод символов
  glfwSetCharCallback(win_main, nullptr);
}


///
/// \brief wglfw::cursor_restore
///
void trgl::cursor_restore(void)
{
  glfwSetInputMode(win_main, GLFW_CURSOR, GLFW_CURSOR_NORMAL);

  // разрешить ввод символов
  glfwSetCharCallback(win_main, callback_char);
}


///
/// \brief wglfw::set_cursor_pos
/// \param x
/// \param y
///
void trgl::set_cursor_pos(double x, double y)
{
  glfwSetCursorPos(win_main, x, y);
}


///
/// \brief wglfw::get_frame_buffer_size
/// \param width
/// \param height
///
void trgl::get_frame_size(int* width, int* height)
{
  glfwGetFramebufferSize(win_main, width, height);
}


///
/// \brief wglfw::get_win_id
/// \return
///
GLFWwindow* trgl::get_id(void) const
{
  return win_main;
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
void trgl::callback_error(int error, const char* description)
{
  std::string Message = "GLFW error " + std::to_string(error) + ": " + description;
  if(error_observer != nullptr) error_observer->event_error(Message.c_str());
}


///
/// Обработчик перемещений курсора мыши
/// \param ptWin - указатель окна
/// \param xpos  - X координата курсора в окне
/// \param ypos  - Y координата курсора в окне
///
void trgl::callback_cursor(GLFWwindow*, double x, double y)
{
  if(cursor_observer != nullptr) cursor_observer->event_cursor(x, y);
}


///
/// \brief window_glfw::mouse_button_callback
/// \param window
/// \param button
/// \param action
/// \param mods
///
void trgl::callback_button(GLFWwindow*, int button, int action, int mods)
{
  if(button_observer != nullptr) button_observer->event_mouse_btns(button, action, mods);
}


///
/// Keys events callback
///
void trgl::callback_keyboard(GLFWwindow*, int key, int scancode, int action, int mods)
{
  if(keyboard_observer != nullptr) keyboard_observer->event_keyboard(key, scancode, action, mods);
}


///
/// GLFW window moving callback
///
void trgl::callback_position(GLFWwindow*, int left, int top)
{
  if(position_observer != nullptr) position_observer->event_reposition(left, top);
}


///
/// \brief wglfw::resize_callback
/// \param WindowPointer
/// \param width
/// \param height
/// \details GLFW framebuffer and window-data callback resize
///
void trgl::callback_size(GLFWwindow*, int width, int height)
{
  for(auto& observer: size_observers) observer->event_resize(width, height);
}


///
/// \brief glfw_wr::character_callback
/// \param window
/// \param key
///
void trgl::callback_char(GLFWwindow*, uint ch)
{
  if(char_observer != nullptr) char_observer->event_character(ch);
}


///
/// \brief wglfw::window_close_callback
/// \param w
///
void trgl::callback_close(GLFWwindow*)
{
  if(close_observer != nullptr) close_observer->event_close();
}


///
/// \brief wglfw::window_focus_callback
/// \param w
///
void trgl::callback_focus(GLFWwindow*,  int focused)
{
  if(0 == focused)
  {
    if(focuslost_observer != nullptr) focuslost_observer->event_focus_lost();
  }
}

} //namespace tr
