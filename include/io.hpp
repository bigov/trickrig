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

namespace tr
{
  struct pngImg
  {
    GLsizei w = 0;
    GLsizei h = 0;
    size_t size = 0;
    std::vector<unsigned char> img {};
  };

  extern pngImg get_png_img(const std::string & filename);
  extern std::vector<char> get_txt_chars(const std::string &);
  extern int sh2int(short, short);
  extern void info(const std::string &);
  extern int get_msec(void);
  extern int random_int(void);
  extern short random_short(void);
  inline float fround( float x) { return static_cast<float>(round(x)); }

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
    f3d(glm::vec3 v): x(v[0]), y(v[1]), z(v[2]) {}
  };

  extern bool operator< (f3d const& left, f3d const& right);

  struct c3d { char x = 0, y = 0, z = 0;  };
  extern bool operator< (c3d const& left, c3d const& right);

  struct f2d { float x = 0.f, z = 0.f; };
  extern bool operator< (f2d const& left, f2d const& right);
}

#endif //__IO_HPP__
