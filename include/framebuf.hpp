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
  unsigned int object_id = 0;
  unsigned int draw_id = 0;
  unsigned int primitive_id = 0;
};


class fb_ren
{
private:
  GLuint id = 0;
  GLuint rbuf_id = 0;
  GLuint tex_space_id  = 0;    // id тектуры для рендера фрейм-буфера

public:
  fb_ren(void) {}
  ~fb_ren(void);

  bool init(void);
  void resize(GLsizei w, GLsizei h);
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
