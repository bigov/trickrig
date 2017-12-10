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

class VBO {
  private:
    GLuint id = 0;            // индекс VBO
    GLsizeiptr allocated = 0; // (максимальный) выделяемый размер буфера
    GLsizeiptr hem = 0;       // граница текущего блока данных в VBO
    GLenum gl_buffer_type;

  public:
    VBO(GLenum type): gl_buffer_type(type) {}
    ~VBO(void) {}
    void Allocate(GLsizeiptr allocated);
    void Allocate(GLsizeiptr allocated, const GLvoid* data);
    void Attrib(GLuint, GLint, GLenum, GLboolean, GLsizei, const GLvoid*);
    void AttribI(GLuint, GLint, GLenum, GLsizei, const GLvoid*);
    void Reduce(GLintptr, GLintptr, GLsizeiptr);
    GLsizeiptr SubDataAppend(GLsizeiptr data_size, const GLvoid* data);
    void SubDataUpdate(GLsizeiptr, const GLvoid*, GLsizeiptr offset);
    void Resize(GLsizeiptr new_hem);
};

} //tr
#endif

