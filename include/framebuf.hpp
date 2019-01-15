/*
 * filename: frbuf.hpp
 *
 * Заголовок класса управления фрейм-буфером
 */

#ifndef FRBUF_HPP
#define FRBUF_HPP

#include "main.hpp"
#include "io.hpp"

namespace tr
{

struct pixel_info
{
  unsigned int r = 0;
  unsigned int g = 0;
  unsigned int b = 0;
};


class fb_base
{
private:
  GLuint id = 0;
  GLuint rbuf_id = 0;
  GLuint tex_color = 0;  // тектура рендера пространства
  GLuint tex_ident = 0;  // тектуры идентификации объетов

public:
  fb_base(void) {}
  ~fb_base(void);

  bool init(GLsizei w, GLsizei h);
  void resize(GLsizei w, GLsizei h);
  pixel_info read_pixel(GLint x, GLint y);
  void bind(void);
  void unbind(void);
};


///
/// \brief The Framebuffer class
///
class fb_tex
{
private:
  GLuint id = 0;
  GLuint tex_color_id = 0;
  GLuint tex_depth_id = 0;

public:
  fb_tex(void) {}
  ~fb_tex(void);

  bool init(GLsizei width, GLsizei height);
  void resize(GLsizei width, GLsizei height);
  void bind(void);
  void unbind(void);
  pixel_info read_pixel(GLint x, GLint y);
};

} //namespace
#endif // FRBUF_HPP
