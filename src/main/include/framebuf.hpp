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

class gl_texture
{
  private:
    GLuint texture_id = 0;

    GLint level = 0, border = 0;
    GLenum target = GL_TEXTURE_2D;
    GLenum texture_num;
    GLint internalformat;
    GLenum format;
    GLenum type;

  public:
    gl_texture(GLenum texture_num, GLint internalformat, GLenum format, GLenum type,
               GLsizei width = 0, GLsizei height = 0, const GLvoid* data = nullptr);
    ~gl_texture() {}

    void bind(void);
    void unbind(void);
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

  // Параметры буфера идентификации примитива
  GLenum ident_format = GL_RED_INTEGER;   // параметры format и type используются
  GLenum ident_type = GL_INT;             // еще и в frame_buffer::read_pixel


public:
  frame_buffer(GLsizei w, GLsizei h);
  ~frame_buffer(void);

  void resize(int w, int h);
  void read_pixel(GLint screen_coord_x, GLint screen_coord_y, void* pixel_data);
  void bind(void);
  void unbind(void);
};

} //namespace
#endif // FRBUF_HPP
