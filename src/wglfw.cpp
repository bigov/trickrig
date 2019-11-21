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
std::string wglfw::title = "TrickRig: v.development";
double wglfw::half_w = 0.0;   // середина окна по X
double wglfw::half_h = 0.0;   // середина окна по Y
bool wglfw::sight_mode = false;

IWindowInput* wglfw::error_observer = nullptr;
IWindowInput* wglfw::cursor_observer = nullptr;
IWindowInput* wglfw::button_observer = nullptr;
IWindowInput* wglfw::keyboard_observer = nullptr;
IWindowInput* wglfw::position_observer = nullptr;
std::list<IWindowInput*> wglfw::size_observers {};
IWindowInput* wglfw::char_observer = nullptr;
IWindowInput* wglfw::close_observer = nullptr;

///
/// Создание нового окна с обработчиками ввода и настройка контекста
/// отображения OpenGL
///
wglfw::wglfw(int width, int height, int min_w, int min_h, int left, int top)
{
  glfwSetErrorCallback(callback_error);
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
  glfwSetKeyCallback(win_ptr, callback_keyboard);
  glfwSetCharCallback(win_ptr, callback_char);
  glfwSetMouseButtonCallback(win_ptr, callback_button);
  glfwSetCursorPosCallback(win_ptr, callback_cursor);
  glfwSetFramebufferSizeCallback(win_ptr, callback_size);
  glfwSetWindowPosCallback(win_ptr, callback_position);
  glfwSetWindowCloseCallback(win_ptr, callback_close);

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


void wglfw::set_error_observer(IWindowInput& ref)
{
  error_observer = &ref;
}
void wglfw::set_cursor_observer(IWindowInput& ref)
{
  cursor_observer = &ref;
}
void wglfw::set_button_observer(IWindowInput& ref)
{
  button_observer = &ref;
}
void wglfw::set_keyboard_observer(IWindowInput& ref)
{
  keyboard_observer = &ref;
}
void wglfw::set_position_observer(IWindowInput& ref)
{
  position_observer = &ref;
}
void wglfw::add_size_observer(IWindowInput& ref)
{
  size_observers.push_back(&ref);
}
void wglfw::set_char_observer(IWindowInput& ref)
{
  char_observer = &ref;
}
void wglfw::set_close_observer(IWindowInput& ref)
{
  close_observer = &ref;
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
  sight_mode = true; // активировать режим прицеливания
  glfwSetInputMode(win_ptr, GLFW_CURSOR, GLFW_CURSOR_HIDDEN);
  glfwSetCursorPos(win_ptr, half_w, half_h); // курсор в центр окна

  // отключить ввод символов
  glfwSetCharCallback(win_ptr, nullptr);
}


///
/// \brief wglfw::cursor_restore
///
void wglfw::cursor_restore(void)
{
  sight_mode = false; // восстановить курсор мыши
  glfwSetInputMode(win_ptr, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
  glfwSetCursorPos(win_ptr, half_w, half_h); // Установить курсор в центре окна

  // разрешить ввод символов
  glfwSetCharCallback(win_ptr, callback_char);
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

  // В режиме прицеливания вернуть курсор в центр окна
  if(sight_mode) glfwSetCursorPos(w, half_w, half_h);
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
  glViewport(0, 0, width, height); // пересчет Viewport
  half_w = width / 2.0;            // пересчет центра окна
  half_h = height / 2.0;
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
  if(position_observer != nullptr) position_observer->close_event();
}

} //namespace tr
