#ifndef GUI_HPP
#define GUI_HPP

#include "main.hpp"
#include "tools.hpp"

namespace tr
{
  enum HALIGN { LEFT, CENTER, RIGHT };
  enum VALIGN { TOP, MIDDLE, BOTTOM};
  enum STATE { ST_NORMAL, ST_OVER, ST_PRESSED, ST_INACTIVE };


  class element
  {
    protected:
      px BgColor { 0xFF, 0xFF, 0xFF, 0xFF };
      STATE state = ST_NORMAL;
      image Image {};

    public:
      element(void) = default;
      element(unsigned int width, unsigned int height, px Color = { 0xFF, 0xFF, 0xFF, 0xFF });
      ~element(void) = default;

      auto draw (STATE new_state = ST_NORMAL);
      void resize (unsigned int new_width, unsigned int new_height);
      uchar* data(void);
      void put(image& TargetImage, uint X, uint Y) { Image.put(TargetImage, X, Y); };
      auto get_width(void) const { return Image.get_width(); }
      auto get_height(void) const { return Image.get_height(); }

      //void put(element E, uint X, uint Y)

      //void insert(const element& E, uint left, uint top);
  };


  class label: public element
  {
    public:
      label(const std::string& new_text);

    protected:
      unsigned int font_id = 0;
      unsigned int font_height = 12;

      std::string text {};

      HALIGN text_halign = CENTER;
      VALIGN text_valign = MIDDLE;
      px text_color { 0x00, 0x00, 0x00, 0xFF };
      unsigned int text_width = 0;
  };
}

#endif // GUI_HPP
