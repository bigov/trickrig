/*
 * filename: frbuf.hpp
 *
 * Заголовок класса управления фрейм-буфером
 */

#ifndef FRBUF_HPP
#define FRBUF_HPP

#include "main.hpp"

namespace tr
{

struct pixel_info
{
  unsigned int object_id = 0;
  unsigned int draw_id = 0;
  unsigned int primitive_id = 0;
};

///
/// \brief The Framebuffer class
///
class framebuf
{

private:
  GLuint id = 0;
  GLuint tex_color_id = 0;
  GLuint tex_depth_id = 0;

public:
  framebuf(void) {}
  ~framebuf(void);

  bool init(GLsizei width, GLsizei height);
  void resize(GLsizei width, GLsizei height);
  void bind(void);
  void unbind(void);
  pixel_info read_pixel(GLint x, GLint y);
};

} //namespace
#endif // FRBUF_HPP
