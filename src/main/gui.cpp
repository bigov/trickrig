#include "gui.hpp"

namespace tr
{

  ///
  /// \brief element::element
  /// \param width
  /// \param height
  ///
  element::element(unsigned int width, unsigned int height)
  {
    resize(width, height);
  }


  ///
  /// \brief element::resize
  /// \param new_width
  /// \param new_height
  ///
  void element::resize(unsigned int new_width, unsigned int new_height)
  {
    width = new_width;
    height = new_height;
    draw(state);
  }


  ///
  /// \brief element::draw
  /// \param new_state
  ///
  void element::draw(STATE new_state)
  {
    state = new_state;
    Data.clear();
    Data.resize(width * height * 4, 0xFF);
  }

  ///
  /// \brief element::data
  /// \return
  ///
  uchar* element::data(void)
  {
    return Data.data();
  }

}
