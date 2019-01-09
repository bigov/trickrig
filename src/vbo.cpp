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

///
/// \brief vbo::shrink
/// \param delta
///
/// \details Сжатие границы размещения активных данных в буфере
///
void vbo::shrink(GLsizeiptr delta)
{
  if(0 == delta) return;
  if(delta > hem) ERR("VBO::shrink got negative value of new size");
  hem -= delta;
}

  ///
  /// \brief vbo::allocate
  /// \param al
  ///
  /// \details Cоздание нового буфера указанного в параметре размера
  ///
  void vbo::allocate(GLsizeiptr al)
  {
    if(0 != id) ERR("VBO::Allocate trying to re-init exist object.");
    allocated = al;
    glGenBuffers(1, &id);
    glBindBuffer(gl_buffer_type, id);
    glBufferData(gl_buffer_type, allocated, nullptr, GL_STATIC_DRAW);

    #ifndef NDEBUG //--контроль создания буфера--------------------------------
    GLint d_size = 0;
    glGetBufferParameteriv(gl_buffer_type, GL_BUFFER_SIZE, &d_size);
    assert(allocated == d_size);
    #endif //------------------------------------------------------------------
    
    if(GL_ARRAY_BUFFER == gl_buffer_type) glBindBuffer(GL_ARRAY_BUFFER, 0);
    hem = 0;
  }


  ///
  /// \brief vbo::allocate
  /// \param al
  /// \param data
  ///
  /// \details Cоздание графического буфера и заполнение его данными
  ///
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
  }


  ///
  /// \brief vbo::attrib
  /// \param index
  /// \param d_size
  /// \param type
  /// \param normalized
  /// \param stride
  /// \param pointer
  ///
  /// \details Настройка атрибутов для float
  ///
  void vbo::attrib(GLuint index, GLint d_size, GLenum type, GLboolean normalized,
    GLsizei stride, size_t pointer)
  {
    glEnableVertexAttribArray(index);
    glBindBuffer(gl_buffer_type, id);
    glVertexAttribPointer(index, d_size, type, normalized, stride,
                          reinterpret_cast<void*>(pointer));
    glBindBuffer(gl_buffer_type, 0);
  }


  ///
  /// \brief vbo::attrib_i
  /// \param index
  /// \param d_size
  /// \param type
  /// \param stride
  /// \param pointer
  ///
  /// \details  Настройка атрибутов для int
  ///
  void vbo::attrib_i(GLuint index, GLint d_size, GLenum type,
    GLsizei stride, const GLvoid* pointer)
  {
    glEnableVertexAttribArray(index);
    glBindBuffer(gl_buffer_type, id);
    glVertexAttribIPointer(index, d_size, type, stride, pointer);
    glBindBuffer(gl_buffer_type, 0);
  }

  ///
  /// \brief vbo::jam_data
  /// \param src
  /// \param dst
  /// \param d_size
  ///
  /// \details Сжатие буфера атрибутов за счет перемещения блока данных
  /// из хвоста. Данные перемещаются внутри VBO только из хвоста ближе
  /// к началу на освободившее место. Адрес начала свободного блока
  /// берется из кэша.
  ///
  void vbo::jam_data(GLintptr src, GLintptr dst, GLsizeiptr d_size)
  {
    #ifndef NDEBUG // контроль направления переноса - только крайний в конце блок
      if((src + d_size) != hem) ERR("vbo::jam_data got err src address");
      if(dst > (src - d_size))  ERR("vbo::jam_data: dst + size > src");
    #endif

    if(GL_ARRAY_BUFFER == gl_buffer_type) glBindBuffer(GL_ARRAY_BUFFER, id);
    glCopyBufferSubData(gl_buffer_type, gl_buffer_type, src, dst, d_size);
    if(GL_ARRAY_BUFFER == gl_buffer_type) glBindBuffer(GL_ARRAY_BUFFER, 0);
    hem = src;
  }


  ///
  /// \brief vbo::data_append
  /// \param d_size
  /// \param data
  /// \return
  ///
  /// \details Добавление данных в конец VBO (с контролем границы размера
  /// буфера). Вносит данные в буфер по указателю границы данных блока (hem),
  /// возвращает положение указателя по которому разместились данные и
  /// сдвигает границу указателя на размер внесенных данных для приема
  /// следующеего блока данных
  ///
  GLsizeiptr vbo::data_append(GLsizeiptr d_size, const GLvoid* data)
  {
    #ifndef NDEBUG // проверка свободного места в буфере----------------------
    if((allocated - hem) < d_size) ERR("VBO::SubDataAppend got overflow buffer");
    #endif //------------------------------------------------------------------

    if(GL_ARRAY_BUFFER != gl_buffer_type) return 0;

    glBindBuffer(GL_ARRAY_BUFFER, id);
    glBufferSubData(GL_ARRAY_BUFFER, hem, d_size, data);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    GLsizeiptr res = hem;
    hem += d_size;

    return res;
  }


  ///
  /// Добавление данных в конец VBO (с контролем границы размера буфера)
  ///
  /// вносит данные в буфер по указателю границы данных блока (hem) без фиксации
  /// положения добавленых данных. Используется для отображения полупрозрачной
  /// области над выделенным снипом.
  ///
  void vbo::data_append_tmp(GLsizeiptr d_size, const GLvoid* data)
  {
    #ifndef NDEBUG // проверка свободного места в буфере----------------------
    if((allocated - hem) < d_size) ERR("VBO::SubDataAppend got overflow buffer");
    #endif //------------------------------------------------------------------

    if(GL_ARRAY_BUFFER != gl_buffer_type) return;
    glBindBuffer(GL_ARRAY_BUFFER, id);
    glBufferSubData(GL_ARRAY_BUFFER, hem, d_size, data);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
  }


  ///
  /// \brief vbo::data_update
  /// \param d_size
  /// \param data
  /// \param dst
  /// \return
  /// \details Замена блока данных в указанном месте (с контролем положения)
  ///
  bool vbo::data_update(GLsizeiptr d_size, const GLvoid* data, GLsizeiptr dst)
  {
    if(dst > (hem - d_size)) return false; // протухший адрес из кэша

    if(GL_ARRAY_BUFFER == gl_buffer_type) glBindBuffer(GL_ARRAY_BUFFER, id);
    glBufferSubData(gl_buffer_type, dst, d_size, data);
    if(GL_ARRAY_BUFFER == gl_buffer_type) glBindBuffer(GL_ARRAY_BUFFER, 0);
    return true;
  }

} //tr
