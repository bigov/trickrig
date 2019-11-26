/*
 * filename: framebuf.cpp
 *
 * Класс управления фрейм-буфером
 */

#include "framebuf.hpp"

namespace tr
{
///
/// \brief gl_texture::gl_texture
/// \details
///
/// void glTexImage2D(GLenum target, GLint level, GLint internalformat,
///                   GLsizei width, GLsizei height, GLint border,
///                   GLenum format, GLenum type, const GLvoid * data);
///
gl_texture::gl_texture( GLint internalformat, GLenum format, GLenum type,
                       GLsizei width, GLsizei height, const GLvoid* data)
  : internalformat(internalformat), format(format), type(type), width(width), height(height)
{
  glGenTextures(1, &texture_id);
  glBindTexture(target, texture_id);
  glTexParameteri(target, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  glTexParameteri(target, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  glTexImage2D(target, level, internalformat, width, height, border, format, type, data);
}


///
/// \brief gl_texture::resize
/// \param width
/// \param height
/// \param data
/// \details Изменение размера памяти под текстуру. Инициалиированную область
/// можно заполнить изображением в формате RGBA - адрес данных изображения надо
/// передать в третьем параметре. Иначе область заливается ровным голубым цветом.
///
void gl_texture::resize(GLsizei width, GLsizei height, const GLvoid* data)
{
  glBindTexture(target, texture_id);

  if(data == nullptr)
  {
    img Blue{static_cast<u_long>(width), static_cast<u_long>(height),
      {0x7F, 0xB0, 0xFF, 0xFF}};  // голубой цвет (формат RGBA)
    glTexImage2D(target, level, internalformat, width, height, border, format, type, Blue.uchar());
  } else {
    glTexImage2D(target, level, internalformat, width, height, border, format, type, data);
  }

  glBindTexture(target, 0);
}


///
/// \brief gl_texture::id
/// \return
///
GLuint gl_texture::id(void) const
{
  return texture_id;
}


///
/// \brief frame_buffer::init
/// \param GLsizei w, GLsizei h
/// \return
///
bool frame_buffer::init(GLsizei w, GLsizei h)
{
  glGenFramebuffers(1, &id);
  glBindFramebuffer(GL_FRAMEBUFFER, id);

  // настройка текстуры для рендера 3D пространства
  glActiveTexture(GL_TEXTURE1);
  TexColor = std::make_unique<gl_texture>(GL_RGBA, GL_RGBA, GL_UNSIGNED_BYTE);
  glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, TexColor->id(), 0);

  // настройка текстуры для идентификации примитивов
  glActiveTexture(GL_TEXTURE2);

  ident_format = GL_RED_INTEGER;   // параметры format и type используются
  ident_type = GL_INT;             // еще и в frame_buffer::read_pixel
  TexIdent = std::make_unique<gl_texture>(GL_R32I, ident_format, ident_type);
  glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, GL_TEXTURE_2D, TexIdent->id(), 0);

  GLenum b[] = { GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1 };
  glDrawBuffers(2, b);

  glGenRenderbuffers(1, &rbuf_id);             // рендер-буфер (глубина и стенсил)
  glBindRenderbuffer(GL_RENDERBUFFER, rbuf_id);
  glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, rbuf_id);

  glBindRenderbuffer(GL_RENDERBUFFER, 0);
  glBindFramebuffer(GL_FRAMEBUFFER, 0);

  resize(w, h);
  return glGetError() == GL_NO_ERROR;
}


///
/// \brief fb_ren::resize
/// \param width
/// \param height
///
void frame_buffer::resize(int w, int h)
{
  glViewport(0, 0, w, h); // пересчет Viewport

  TexIdent->resize(w, h); // Текстура индентификации примитивов (канал RED)
  TexColor->resize(w, h);

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
void frame_buffer::read_pixel(GLint x, GLint y, void* pixel_data)
{
  assert((x >= 0) && (y >= 0));

  glBindFramebuffer(GL_READ_FRAMEBUFFER, id);
  glReadBuffer(GL_COLOR_ATTACHMENT1);
  glReadPixels(x, y, 1, 1, ident_format, ident_type, pixel_data);
  glReadBuffer(GL_NONE);
  glBindFramebuffer(GL_READ_FRAMEBUFFER, 0);
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
