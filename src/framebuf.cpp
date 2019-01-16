/*
 * filename: framebuf.cpp
 *
 * Класс управления фрейм-буфером
 */

#include "framebuf.hpp"

namespace tr
{

///
/// \brief fb_ren::init
/// \param texture_id
/// \return
///
bool frame_buffer::init(GLsizei w, GLsizei h, GLint gl_tex_color, GLint gl_tex_ident)
{
  //gl_tex_color = GL_TEXTURE1;
  //gl_tex_ident = GL_TEXTURE2;

  glGenFramebuffers(1, &id);
  glBindFramebuffer(GL_FRAMEBUFFER, id);

  // настройка текстуры для рендера 3D пространства
  glActiveTexture(gl_tex_color);
  glGenTextures(1, &tex_color);
  glBindTexture(GL_TEXTURE_2D, tex_color);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, w, h, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
  glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, tex_color, 0);

  glActiveTexture(gl_tex_ident);
  glGenTextures(1, &tex_ident);
  glBindTexture(GL_TEXTURE_2D, tex_ident);
  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB32UI, w, h, 0, GL_RGB_INTEGER, GL_UNSIGNED_INT, nullptr);
  glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, GL_TEXTURE_2D, tex_ident, 0);

  GLenum  b[] = { GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1 };
  glDrawBuffers(2, b);

  glGenRenderbuffers(1, &rbuf_id);             // рендер-буфер (глубина и стенсил)
  glBindRenderbuffer(GL_RENDERBUFFER, rbuf_id);
  glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, rbuf_id);

  glBindTexture(GL_TEXTURE_2D, 0);
  glBindRenderbuffer(GL_RENDERBUFFER, 0);
  glBindFramebuffer(GL_FRAMEBUFFER, 0);

  return glGetError() == GL_NO_ERROR;
}


///
/// \brief fb_ren::resize
/// \param width
/// \param height
///
void frame_buffer::resize(GLsizei w, GLsizei h)
{
  GLint lod = 0, frame = 0;
  img Blue{static_cast<u_long>(w), static_cast<u_long>(h), {0x7F, 0xB0, 0xFF, 0xFF}};  // голубой цвет

  // Настройка размера текстуры рендера идентификации
  glBindTexture(GL_TEXTURE_2D, tex_ident);
  glTexImage2D(GL_TEXTURE_2D, lod, GL_RGB32UI, w, h, frame, GL_RGB_INTEGER, GL_UNSIGNED_INT, nullptr);
  glBindTexture(GL_TEXTURE_2D, 0);

  // Настройка размера и заливка фоновым цветом текстуры рендера фреймбуфера
  glBindTexture(GL_TEXTURE_2D, tex_color);
  glTexImage2D(GL_TEXTURE_2D, lod, GL_RGBA, w, h, frame, GL_RGBA, GL_UNSIGNED_BYTE, Blue.uchar());
  glBindTexture(GL_TEXTURE_2D, 0);

  // настройка размера рендербуфера
  glBindRenderbuffer(GL_RENDERBUFFER, rbuf_id);
  glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, w, h);
  glBindRenderbuffer(GL_RENDERBUFFER, 0);
}


///
/// \brief fb_ren::bind
///
void frame_buffer::bind(void)
{
  glBindFramebuffer(GL_FRAMEBUFFER, id);
  glBindRenderbuffer(GL_RENDERBUFFER, rbuf_id);
}


///
/// \brief framebuf::read_pixel
/// \param x
/// \param y
/// \return
///
pixel_info frame_buffer::read_pixel(GLint x, GLint y)
{
  glBindFramebuffer(GL_READ_FRAMEBUFFER, id);
  glReadBuffer(GL_COLOR_ATTACHMENT1);
  pixel_info Pixel;
  glReadPixels(x, y, 1, 1, GL_RGB_INTEGER, GL_UNSIGNED_INT, &Pixel);

  glReadBuffer(GL_NONE);
  glBindFramebuffer(GL_READ_FRAMEBUFFER, 0);

  return Pixel;
}


///
/// \brief fb_ren::unbind
///
void frame_buffer::unbind(void)
{
  glBindTexture(GL_TEXTURE_2D, 0);
  glBindRenderbuffer(GL_RENDERBUFFER, 0);
  glBindFramebuffer(GL_FRAMEBUFFER, 0);
}


///
/// \brief fb_ren::~fb_ren
///
frame_buffer::~frame_buffer(void)
{
  unbind();
  glDeleteFramebuffers(1, &id);
}


}
