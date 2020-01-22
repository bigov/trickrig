/*
 * filename: frbuf.hpp
 *
 * Заголовок класса управления фрейм-буфером
 */

#ifndef FRBUF_HPP
#define FRBUF_HPP

#include "io.hpp"

namespace tr
{

class gl_texture
{
  private:
    GLuint texture_id = 0;

    GLint level = 0, border = 0;
    GLenum target = GL_TEXTURE_2D;
    GLint internalformat;
    GLenum format;
    GLenum type;

  public:
    gl_texture(GLint internalformat, GLenum format, GLenum type,
               GLsizei width = 0, GLsizei height = 0, const GLvoid* data = nullptr);
    ~gl_texture() {}

    void resize(GLsizei width, GLsizei height, const GLvoid* data = nullptr);
    GLuint id(void) const;
};

class frame_buffer
{
private:
  GLuint id = 0;
  GLuint rbuf_id = 0;
  std::unique_ptr<gl_texture> TexColor = nullptr;
  std::unique_ptr<gl_texture> TexIdent = nullptr;

  GLenum ident_format = 0;
  GLenum ident_type = 0;

public:
  frame_buffer(void) {}
  ~frame_buffer(void);

  bool init(GLsizei w, GLsizei h);
  void resize(int w, int h);
  void read_pixel(GLint screen_coord_x, GLint screen_coord_y, void* pixel_data);
  void bind(void);
  void unbind(void);
};

} //namespace
#endif // FRBUF_HPP
