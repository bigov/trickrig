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
bool fb_ren::init(void)
{

  // настройка текстуры для рендера 3D пространства
  glActiveTexture(GL_TEXTURE1);
  glGenTextures(1, &tex_space_id);
  glBindTexture(GL_TEXTURE_2D, tex_space_id);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);


  glGenFramebuffers(1, &id);
  glBindFramebuffer(GL_FRAMEBUFFER, id);
  glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, tex_space_id, 0);

  glGenRenderbuffers(1, &rbuf_id);             // рендер-буфер (глубина и стенсил)
  glBindRenderbuffer(GL_RENDERBUFFER, rbuf_id);
  glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, rbuf_id);

  glBindRenderbuffer(GL_RENDERBUFFER, 0);
  glBindFramebuffer(GL_FRAMEBUFFER, 0);

  return glGetError() == GL_NO_ERROR;
}


///
/// \brief fb_ren::bind
///
void fb_ren::bind(void)
{
  glBindFramebuffer(GL_FRAMEBUFFER, id);
}


///
/// \brief fb_ren::unbind
///
void fb_ren::unbind(void)
{
  glBindFramebuffer(GL_FRAMEBUFFER, 0);
}


///
/// \brief fb_ren::resize
/// \param width
/// \param height
///
void fb_ren::resize(GLsizei w, GLsizei h)
{
  GLint l_o_d = 0, frame = 0;

  // пересчет Viewport
  glViewport(0, 0, w, h);

  // настройка размера текстуры рендера фреймбуфера
  glBindTexture(GL_TEXTURE_2D, tex_space_id);
  glTexImage2D(GL_TEXTURE_2D, l_o_d, GL_RGBA, w, h, frame, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);

  // настройка размера рендербуфера
  glBindRenderbuffer(GL_RENDERBUFFER, rbuf_id);
  glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, w, h);
}


///
/// \brief fb_ren::~fb_ren
///
fb_ren::~fb_ren(void)
{
  unbind();
  glDeleteFramebuffers(1, &id);
}


///
/// \brief framebuf::init
/// \param width
/// \param height
/// \return
///
bool fb_tex::init(GLsizei width, GLsizei height)
{
  glGenFramebuffers(1, &id);
  glBindFramebuffer(GL_DRAW_FRAMEBUFFER, id);

  // Create the texture object for the primitive information buffer
  glGenTextures(1, &tex_color_id);
  glBindTexture(GL_TEXTURE_2D, tex_color_id);
  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB32UI, width, height, 0, GL_RGB_INTEGER, GL_UNSIGNED_INT, nullptr);

  glFramebufferTexture2D(GL_DRAW_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, tex_color_id, 0);

  // Create the texture object for the depth buffer
  glGenTextures(1, &tex_depth_id);
  glBindTexture(GL_TEXTURE_2D, tex_depth_id);
  glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, width, height, 0, GL_DEPTH_COMPONENT, GL_FLOAT,  nullptr);
  glFramebufferTexture2D(GL_DRAW_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, tex_depth_id, 0);

  //
  //TODO
  // 1. настройку размеров текстур сделать через вызов метода resize(w, h)
  // 2. проверить, какой параметр д.б. в glCheckFramebufferStatus( ??? )
  //    GL_FRAMEBUFFER или GL_DRAW_FRAMEBUFFER

  GLenum Status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
  if (Status != GL_FRAMEBUFFER_COMPLETE) ERR("Framebufer error, status: " + std::to_string(Status));

  glBindTexture(GL_TEXTURE_2D, 0);
  glBindFramebuffer(GL_FRAMEBUFFER, 0);
  return glGetError() == GL_NO_ERROR;
}


///
/// \brief framebuf::resize
/// \param width
/// \param height
///
void fb_tex::resize(GLsizei width, GLsizei height)
{
  GLint lod = 0, frame = 0;
  glBindFramebuffer(GL_DRAW_FRAMEBUFFER, id);

  glBindTexture(GL_TEXTURE_2D, tex_color_id);
  glTexImage2D(GL_TEXTURE_2D, lod, GL_RGB32UI, width, height, frame, GL_RGB_INTEGER, GL_UNSIGNED_INT, nullptr);

  glBindTexture(GL_TEXTURE_2D, tex_depth_id);
  glTexImage2D(GL_TEXTURE_2D, lod, GL_DEPTH_COMPONENT, width, height, frame, GL_DEPTH_COMPONENT, GL_FLOAT,  nullptr);

  glBindTexture(GL_TEXTURE_2D, 0);
  glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

///
/// \brief framebuf::bind
///
void fb_tex::bind()
{
  glBindFramebuffer(GL_DRAW_FRAMEBUFFER, id);
}


///
/// \brief framebuf::unbind
///
void fb_tex::unbind()
{
  glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
}


///
/// \brief framebuf::read_pixel
/// \param x
/// \param y
/// \return
///
pixel_info fb_tex::read_pixel(GLint x, GLint y)
{
  glBindFramebuffer(GL_READ_FRAMEBUFFER, id);
  glReadBuffer(GL_COLOR_ATTACHMENT0);
  pixel_info Pixel;
  glReadPixels(x, y, 1, 1, GL_RGB_INTEGER, GL_UNSIGNED_INT, &Pixel);
  glReadBuffer(GL_NONE);
  glBindFramebuffer(GL_READ_FRAMEBUFFER, 0);
  return Pixel;
}


///
/// \brief framebuf::~framebuf
///
fb_tex::~fb_tex()
{
  glBindFramebuffer(GL_DRAW_FRAMEBUFFER, id);

  glBindTexture(GL_TEXTURE_2D, 0);
  if(tex_color_id != 0) glDeleteTextures(1, &tex_color_id);
  tex_color_id = 0;
  if(tex_depth_id != 0) glDeleteTextures(1, &tex_depth_id);
  tex_depth_id = 0;

  glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
  if(id != 0) glDeleteFramebuffers(1, &id);
  id = 0;
}

}
