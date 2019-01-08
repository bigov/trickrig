/*
 * filename: framebuf.cpp
 *
 * Класс управления фрейм-буфером
 */

#include "framebuf.hpp"

namespace tr
{

///
/// \brief framebuf::~framebuf
///
framebuf::~framebuf()
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


///
/// \brief framebuf::init
/// \param width
/// \param height
/// \return
///
bool framebuf::init(GLsizei width, GLsizei height)
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
  //TODO настройку размеров текстур сделать через вызов метода resize(w, h)
  //

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
void framebuf::resize(GLsizei width, GLsizei height)
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
void framebuf::bind()
{
  glBindFramebuffer(GL_DRAW_FRAMEBUFFER, id);
}


///
/// \brief framebuf::unbind
///
void framebuf::unbind()
{
  glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
}


///
/// \brief framebuf::read_pixel
/// \param x
/// \param y
/// \return
///
pixel_info framebuf::read_pixel(GLint x, GLint y)
{
  glBindFramebuffer(GL_READ_FRAMEBUFFER, id);
  glReadBuffer(GL_COLOR_ATTACHMENT0);
  pixel_info Pixel;
  glReadPixels(x, y, 1, 1, GL_RGB_INTEGER, GL_UNSIGNED_INT, &Pixel);
  glReadBuffer(GL_NONE);
  glBindFramebuffer(GL_READ_FRAMEBUFFER, 0);
  return Pixel;
}

}
