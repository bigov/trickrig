#include "winpar.hpp"

namespace tr {

///
/// \brief win_data::error_event
/// \param message
///
void win_params::error_event(const char* message)
{
  info(std::string(message));
}


///
/// \brief ev_input::mouse_event
/// \param _button
/// \param _action
/// \param _mods
///
void win_params::mouse_event(int _button, int _action, int _mods)
{
  mods   = _mods;
  mouse  = _button;
  action = _action;
}


///
/// \brief ev_input::keyboard_event
/// \param key
/// \param scancode
/// \param action
/// \param mods
///
void win_params::keyboard_event(int _key, int _scancode, int _action, int _mods)
{
  mouse    = -1;
  key      = _key;
  scancode = _scancode;
  action   = _action;
  mods     = _mods;
}


///
/// \brief win_data::window_pos_event
/// \param left
/// \param top
///
void win_params::reposition_event(int _left, int _top)
{
  Layout.left = static_cast<u_int>(_left);
  Layout.top = static_cast<u_int>(_top);
}


///
/// \brief gui_window::resize_event
/// \param width
/// \param height
///
void win_params::resize_event(int w, int h)
{
  assert(w >= 0);
  assert(h >= 0);

  Layout.width  = static_cast<u_int>(w);
  Layout.height = static_cast<u_int>(h);

  // пересчет позции координат прицела (центр окна)
  Sight.x = static_cast<float>(w/2);
  Sight.y = static_cast<float>(h/2);

  // пересчет матрицы проекции
  aspect = static_cast<float>(w) / static_cast<float>(h);
  MatProjection = glm::perspective(1.118f, aspect, zNear, zFar);

  // пересчет размеров GUI
  if(nullptr != pWinGui) pWinGui->resize(w, h);
}


///
/// \brief win_data::cursor_position_event
/// \param x
/// \param y
///
void win_params::cursor_event(double x, double y)
{
  xpos = x;
  ypos = y;
}


///
/// \brief win_data::close_event
///
void win_params::close_event(void)
{
  is_open = false;
}


///
/// \brief win_data::set_location
/// \param width
/// \param height
/// \param left
/// \param top
///
void win_params::layout_set(const layout &L)
{
  Layout = L;
  Sight.x = static_cast<float>(L.width/2);
  Sight.y = static_cast<float>(L.height/2);
  aspect  = static_cast<float>(L.width/L.height);
}

}
