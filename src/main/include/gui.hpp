#ifndef GUI_HPP
#define GUI_HPP

#include "main.hpp"
#include "config.hpp"
#include "tools.hpp"

namespace tr
{
  enum HALIGN { LEFT, CENTER, RIGHT };
  enum VALIGN { TOP, MIDDLE, BOTTOM};
  enum STATE { ST_NORMAL, ST_OVER, ST_PRESSED, ST_INACTIVE, ST_COUNT };
  enum FONT_WEIGHT { FONT_NORMAL, FONT_BOLD, FONT_COUNT };


  class element: public image
  {
    protected:
      px BgColor { 0xFF, 0xFF, 0xFF, 0xFF };
      STATE state = ST_NORMAL;

    public:
      element(void) = default;
      element(uint width, uint height, px Color = { 0xFF, 0xFF, 0xFF, 0xFF } );
      void draw (STATE new_state = ST_COUNT);
  };


  class label: public element
  {
    protected:
      unsigned int font_id = 0;
      std::string font_normal = cfg::AssetsDir + cfg::DS + "FreeSans.ttf";
      std::string font_bold = cfg::AssetsDir + cfg::DS + "FreeSansBold.ttf";
      std::string Text {};
      px text_color {88, 88, 88, 0};
      int letter_space = 80; // % size of

    public:
      label(const std::string& new_text, unsigned int new_height = 18,
            FONT_WEIGHT weight = FONT_NORMAL, px new_color = {88, 88, 88, 0});

  };
}

#endif // GUI_HPP
