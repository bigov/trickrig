//============================================================================
//
// file: io.hpp
//
//
//
//============================================================================
#ifndef __IO_HPP__
#define __IO_HPP__

#include "main.hpp"
#include "png.h"
#include "ft2build.h"
#include FT_FREETYPE_H
#include FT_GLYPH_H

namespace tr
{
  struct image
  {
    GLsizei w = 0;
    GLsizei h = 0;
    size_t size = 0;
    std::vector<unsigned char> Data {};
    void flip_vert(void);
  };

  extern image get_png_img(const std::string &filename);
  extern void read_chars_file(const std::string &FNname, std::vector<char> &Buffer);
  extern int sh2int(short, short);
  extern void info(const std::string &);
  extern int get_msec(void);   // число миллисекунд от начала суток.
  extern int random_int(void);
  extern short random_short(void);

  // структура для оперирования с трехмерными координатами
  struct f3d
  {
    float x = 0.f, y = 0.f, z = 0.f;

    // конструкторы для обеспечения инициализации разными типами данных
    f3d(float x, float y, float z): x(x), y(y), z(z) {}
    f3d(double x, double y, double z):
      x(static_cast<float>(x)),
      y(static_cast<float>(y)),
      z(static_cast<float>(z)) {}
    f3d(int x, int y, int z):
      x(static_cast<float>(x)),
      y(static_cast<float>(y)),
      z(static_cast<float>(z)) {}
    f3d(glm::vec3 v): x(v[0]), y(v[1]), z(v[2]) {}
  };

  extern bool operator< (f3d const& left, f3d const& right);

  struct c3d { char x = 0, y = 0, z = 0;  };
  extern bool operator< (c3d const& left, c3d const& right);

  struct f2d { float x = 0.f, z = 0.f; };
  extern bool operator< (f2d const& left, f2d const& right);
}

#endif //__IO_HPP__
