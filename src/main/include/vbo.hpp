/*
 *
 * file: vbo.hpp
 *
 * Header of the GLSL VBOs control class
 *
 */

#ifndef VBO_HPP
#define VBO_HPP

#include "tools.hpp"


namespace tr
{
///
/// \brief The VBO base class
///
class vbo
{
  protected:
    GLenum gl_buffer_type = 0;
    GLuint id = 0;            // индекс VBO
    GLsizeiptr allocated = 0; // (максимальный) выделяемый размер буфера
    GLsizeiptr hem = 0;       // граница размещения данных в VBO

  public:
    vbo(void) = delete;
    vbo(GLenum type);
    vbo(GLenum type, GLuint _id);
    vbo(GLenum t, GLuint i, GLsizeiptr a): gl_buffer_type(t), id(i), allocated(a) {}
    ~vbo(void) {}

    GLuint get_id(void)       { return id; }
    GLsizeiptr get_size(void) { return allocated; }
    GLsizeiptr get_hem(void)  { return hem; }
    GLenum get_type(void)     { return gl_buffer_type; }

    void allocate (GLsizeiptr new_size);
    void allocate (GLsizeiptr new_size, const GLvoid* data);
    void append(const GLsizeiptr data_size, const GLvoid* data);
    GLsizeiptr remove(const GLsizeiptr data_size, const GLsizeiptr dest);
    void update(const GLsizeiptr data_size, const GLvoid* data, GLsizeiptr stride);
    void clear(void) { hem = 0; }

    GLsizeiptr max_size(void);
    void set_attributes (const std::list<glsl_attributes>&);
    void attrib (GLuint, GLint, GLenum, GLboolean, GLsizei, size_t);
    void attrib_i (GLuint, GLint, GLenum, GLsizei, const GLvoid*);
};


///
/// \brief The vbo_ctrl class
/// \details Класс, предназначенный для выполнения в раздельном потоке
/// операций записи с ранее созданным буфером
///
class vbo_ctrl
{
protected:
  GLenum gl_buffer_type;
  GLuint id;              // индекс VBO
  GLsizeiptr allocated;   // (максимальный) выделяемый размер буфера
  GLsizeiptr hem = 0;     // граница размещения данных в VBO
public:
  vbo_ctrl(GLenum t, GLuint i, GLsizeiptr a): gl_buffer_type(t), id(i), allocated(a) {}

  GLsizeiptr append (const GLsizeiptr data_size, const GLvoid* data);
  GLsizeiptr remove (const GLsizeiptr data_size, const GLsizeiptr dest);
};


///
/// \brief The vbo_transit class
/// \details Промежуточный GPU буфер для обмена данными с CPU
///
class vbo_transit: public vbo
{
  protected:
    GLuint id_subbuf = 0;

  public:
    vbo_transit (GLenum type);
    void data_get (GLintptr offset, GLsizeiptr size, GLvoid* data);
};

} //tr
#endif
