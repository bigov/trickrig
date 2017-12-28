//============================================================================
//
// file: vbo.cpp
//
// Class GLSL VBOs control
//
//============================================================================
#include "vbo.hpp"

namespace tr
{
  //## Сжатие границы размещения активных данных в буфере
  void vbo::shrink(GLsizeiptr delta)
  {
    if(0 == delta) return;
    if(delta > hem) ERR("VBO::shrink got negative value of new size");
    hem -= delta;
    return;
  }

  //## Cоздание нового буфера указанного в параметре размера
  void vbo::allocate(GLsizeiptr al)
  {
    if(0 != id) ERR("VBO::Allocate trying to re-init exist object.");
    allocated = al;
    glGenBuffers(1, &id);
    glBindBuffer(gl_buffer_type, id);
    glBufferData(gl_buffer_type, allocated, 0, GL_STATIC_DRAW);

    #ifndef NDEBUG //--контроль создания буфера--------------------------------
    GLint d_size = 0;
    glGetBufferParameteriv(gl_buffer_type, GL_BUFFER_SIZE, &d_size);
    assert(allocated == d_size);
    #endif //------------------------------------------------------------------
    
    if(GL_ARRAY_BUFFER == gl_buffer_type) glBindBuffer(GL_ARRAY_BUFFER, 0);
    hem = 0;
    return;
  }

  //## Cоздание и заполнение буфера данными
  void vbo::allocate(GLsizeiptr al, const GLvoid* data)
  {
    if(0 != id) ERR("VBO::Allocate trying to re-init exist object.");
    allocated = al;
    glGenBuffers(1, &id);
    glBindBuffer(gl_buffer_type, id);
    glBufferData(gl_buffer_type, allocated, data, GL_STATIC_DRAW);

    #ifndef NDEBUG //--контроль создания буфера--------------------------------
    GLint d_size = 0;
    glGetBufferParameteriv(gl_buffer_type, GL_BUFFER_SIZE, &d_size);
    assert(allocated == d_size);
    #endif //------------------------------------------------------------------
    
    if(GL_ARRAY_BUFFER == gl_buffer_type) glBindBuffer(GL_ARRAY_BUFFER, 0);
    hem = al;
    return;
  }

  //## Настройка атрибутов для float
  void vbo::attrib(GLuint index, GLint d_size, GLenum type, GLboolean normalized,
    GLsizei stride, const GLvoid* pointer)
  {
    glEnableVertexAttribArray(index);
    glBindBuffer(gl_buffer_type, id);
    glVertexAttribPointer(index, d_size, type, normalized, stride, pointer);
    glBindBuffer(gl_buffer_type, 0);
    return;
  }

  //## Настройка атрибутов для int
  void vbo::attrib_i(GLuint index, GLint d_size, GLenum type,
    GLsizei stride, const GLvoid* pointer)
  {
    glEnableVertexAttribArray(index);
    glBindBuffer(gl_buffer_type, id);
    glVertexAttribIPointer(index, d_size, type, stride, pointer);
    glBindBuffer(gl_buffer_type, 0);
    return;
  }

  //## Сжатие буфера атрибутов за счет перемещения данных из хвоста
  // в неиспользуемый промежуток и уменьшение текущего индекса
  void vbo::jam_data(GLintptr src, GLintptr dst, GLsizeiptr d_size)
  {
    #ifndef NDEBUG // контроль точки переноса
      if(dst > (src - d_size)) ERR("VBO::CopySubData got err dst");
    #endif

    if(GL_ARRAY_BUFFER == gl_buffer_type) glBindBuffer(GL_ARRAY_BUFFER, id);
    glCopyBufferSubData(gl_buffer_type, gl_buffer_type, src, dst, d_size);
    if(GL_ARRAY_BUFFER == gl_buffer_type) glBindBuffer(GL_ARRAY_BUFFER, 0);
    hem -= d_size;
    return;
  }

  //## Добавление данных в конец (с контролем границы размера буфера)
  GLsizeiptr vbo::data_append(GLsizeiptr d_size, const GLvoid* data)
  {
  /* вносит данные в буфер по указателю границы данных блока (hem), возвращает
   * положение указателя по которому разместились данные и сдвигает границу
   * указателя на размер внесенных данных для следующей порции атрибутов
   */

    #ifndef NDEBUG // проверка свободного места в буфере----------------------
    if((allocated - hem) < d_size) ERR("VBO::SubDataAppend got overflow buffer");
    #endif //------------------------------------------------------------------

    if(GL_ARRAY_BUFFER == gl_buffer_type) glBindBuffer(GL_ARRAY_BUFFER, id);
    glBufferSubData(gl_buffer_type, hem, d_size, data);
    if(GL_ARRAY_BUFFER == gl_buffer_type) glBindBuffer(GL_ARRAY_BUFFER, 0);
    GLsizeiptr res = hem;
    hem += d_size;
    return res;
  }

  //## Замена блока данных в указанном месте (с контролем положения)
  bool vbo::data_update(GLsizeiptr d_size, const GLvoid* data, GLsizeiptr offset)
  {
    if (offset > (hem - d_size)) return false; // Фу, "протухший" адрес!

    if(GL_ARRAY_BUFFER == gl_buffer_type) glBindBuffer(GL_ARRAY_BUFFER, id);
    glBufferSubData(gl_buffer_type, offset, d_size, data);
    if(GL_ARRAY_BUFFER == gl_buffer_type) glBindBuffer(GL_ARRAY_BUFFER, 0);
    return true;
  }

} //tr
