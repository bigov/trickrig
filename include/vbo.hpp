//============================================================================
//
// file: vbo.hpp
//
// Header of the GLSL VBOs control class
//
//============================================================================
#ifndef __VBO_HPP__
#define __VBO_HPP__

#include "main.hpp"
#include "config.hpp"

namespace tr {

class vbo {
  private:
    GLuint id = 0;            // индекс VBO
    GLsizeiptr allocated = 0; // (максимальный) выделяемый размер буфера
    GLsizeiptr hem = 0;       // адрес размещения следующего блока в VBO
    GLenum gl_buffer_type;

  public:
    vbo(GLenum type): gl_buffer_type(type) {}
    ~vbo(void) {}
    void allocate(GLsizeiptr allocated);
    void allocate(GLsizeiptr allocated, const GLvoid* data);
    void attrib(GLuint, GLint, GLenum, GLboolean, GLsizei, const GLvoid*);
    void attrib_i(GLuint, GLint, GLenum, GLsizei, const GLvoid*);
    void jam_data(GLintptr, GLintptr, GLsizeiptr);
    GLsizeiptr data_append(GLsizeiptr data_size, const GLvoid* data);
    bool data_update(GLsizeiptr, const GLvoid*, GLsizeiptr offset);
    void shrink(GLsizeiptr);
    GLsizeiptr get_hem(void) { return hem; }
};

} //tr
#endif

