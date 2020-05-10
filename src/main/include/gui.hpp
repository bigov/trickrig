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
    public:
      label(const std::string& new_text);

    protected:
      unsigned int font_id = 0;
      std::string font_file = cfg::AssetsDir + cfg::DS + "DejaVu Sans Mono for Powerline.ttf";

      unsigned int font_height = 12;

      std::string Text {};

      HALIGN text_halign = CENTER;
      VALIGN text_valign = MIDDLE;
      px text_color { 0x00, 0x00, 0x00, 0xFF };
      unsigned int text_width = 0;
  };
}

#endif // GUI_HPP
