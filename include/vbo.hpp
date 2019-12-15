/*
 *
 * file: vbo.hpp
 *
 * Header of the GLSL VBOs control class
 *
 */

#ifndef VBO_HPP
#define VBO_HPP

#include "io.hpp"

namespace tr
{
///
/// \brief The VBO base class
///
class vbo_base
{
  protected:
    GLenum gl_buffer_type;
    GLuint id = 0;            // индекс VBO
    GLsizeiptr allocated = 0; // (максимальный) выделяемый размер буфера
    GLsizeiptr hem = 0;       // граница размещения данных в VBO

  public:
    vbo_base(GLenum type): gl_buffer_type(type) {}
    ~vbo_base(void) {}

    void allocate (GLsizeiptr allocated);
    GLsizeiptr max_size(void);
    void allocate (GLsizeiptr allocated, const GLvoid* data);
    void set_attributes (const std::list<glsl_attributes>&);
    void attrib (GLuint, GLint, GLenum, GLboolean, GLsizei, size_t);
    void attrib_i (GLuint, GLint, GLenum, GLsizei, const GLvoid*);
    void bind (void);
    void unbind (void);
};


///
/// \brief The VBO extended class
///
class vbo_ext: public vbo_base
{
  protected:
  GLuint id_subbuf = 0; // промежуточный GPU буфер для обмена данными с CPU

  public:
    vbo_ext (GLenum type);
    GLsizeiptr append (const GLvoid* data, GLsizeiptr data_size);
    GLsizeiptr remove (GLsizeiptr dest, GLsizeiptr data_size);
    void data_get (GLintptr offset, GLsizeiptr size, GLvoid* data);
    void clear (void) {hem = 0;}
};

} //tr
#endif
