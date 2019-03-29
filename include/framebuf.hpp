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

class frame_buffer
{
private:
  GLuint id = 0;
  GLuint rbuf_id = 0;
  GLuint tex_color = 0;     // текстура рендера пространства
  GLuint tex_ident = 0;     // текстура идентификации объетов

#ifndef NDEBUG
  GLint fb_w = 0, fb_h = 0; // размеры буфера
#endif

public:
  frame_buffer(void) {}
  ~frame_buffer(void);

  bool init(GLsizei w, GLsizei h);
  void resize(GLsizei w, GLsizei h);
  void read_pixel(GLint screen_coord_x, GLint screen_coord_y, void* pixel_data);
  void bind(void);
  void unbind(void);
};

} //namespace
#endif // FRBUF_HPP
