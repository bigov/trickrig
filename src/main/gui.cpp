#include "gui.hpp"

namespace tr
{

  ///
  /// \brief element::element
  /// \param width
  /// \param height
  ///
  element::element(unsigned int width, unsigned int height, px Color)
  {
    BgColor = Color;
    Image.resize(width, height, BgColor);
  }


  ///
  /// \brief element::resize
  /// \param new_width
  /// \param new_height
  ///
  void element::resize(unsigned int new_width, unsigned int new_height)
  {
    Image.resize(new_width, new_height, BgColor);
  }


  ///
  /// \brief element::draw
  /// \param new_state
  ///
  auto element::draw(STATE new_state)
  {
    state = new_state;
    return data();

  }

  ///
  /// \brief element::data
  /// \return
  ///
  uchar* element::data(void)
  {
    return Image.uchar_t();
  }


  ///
  /// \brief label::label
  ///
  label::label(const std::string& new_text)
  {
    text = new_text;
  }
}
