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
  unsigned int Xid = 0;
  unsigned int Yid = 0;
  unsigned int Zid = 0;
};


class frame_buffer
{
private:
  GLuint id = 0;
  GLuint rbuf_id = 0;
  GLuint tex_color = 0;  // тектура рендера пространства
  GLuint tex_ident = 0;  // тектуры идентификации объетов

public:
  frame_buffer(void) {}
  ~frame_buffer(void);

  bool init(GLsizei w, GLsizei h);
  void resize(GLsizei w, GLsizei h);
  pixel_info read_pixel(GLint x, GLint y);
  void bind(void);
  void unbind(void);
};

} //namespace
#endif // FRBUF_HPP
