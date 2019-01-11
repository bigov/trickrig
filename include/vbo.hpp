/*
 *
 * file: vbo.hpp
 *
 * Header of the GLSL VBOs control class
 *
 */

#ifndef VBO_HPP
#define VBO_HPP

#include "main.hpp"
#include "rig.hpp"

namespace tr{


///
/// \brief The vbo_base class
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

    void allocate(GLsizeiptr allocated);
    void allocate(GLsizeiptr allocated, const GLvoid* data);
    void attrib(GLuint, GLint, GLenum, GLboolean, GLsizei, size_t);
    void attrib_i(GLuint, GLint, GLenum, GLsizei, const GLvoid*);
};


///
/// \brief The vbo_mem class
///
class vbo_mem: public vbo_base
{
  protected:
    void data_update(GLsizeiptr, const GLvoid*, GLsizeiptr offset);
    GLsizeiptr data_append(GLsizeiptr data_size, const GLvoid* data);
    void shrink(GLsizeiptr);
    void jam_data(GLintptr data_src, GLintptr data_dst, GLsizeiptr data_size);

  public:
    vbo_mem(GLenum type): vbo_base(type) {}
};


///
/// \brief The vbo class
///
class vbo: public vbo_mem
{
  private:
    std::unordered_map<GLsizeiptr, snip*> VisibleSnips {}; // Карта размещения cнипов по адресам в VBO
    GLsizeiptr append(GLfloat*, const f3d&);

  public:
    u_int render_points = 0;                               // число точек передаваемых в рендер

    vbo(GLenum type): vbo_mem(type) {}

    // Функции управления данными снипа в буферах VBO данных и VBO индекса
    void update(GLfloat* s_data, const f3d&, GLsizeiptr);
    void data_place(snip& Snip, const f3d&);               // разместить данные в VBO буфере
    void data_remove(std::vector<snip>&);                  // убрать данные из рендера
};

} //tr
#endif

