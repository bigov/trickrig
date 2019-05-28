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

void vbo_base::bind(void)
{
  glBindBuffer(gl_buffer_type, id);
}

void vbo_base::unbind(void)
{
  glBindBuffer(gl_buffer_type, 0);
}

///
/// \brief vbo::allocate
/// \param al
///
/// \details Cоздание нового буфера указанного в параметре размера
///
void vbo_base::allocate(GLsizeiptr need_size)
{
  if(0 != id) ERR("VBO::Allocate trying to re-init exist object.");
  allocated = need_size;
  glGenBuffers(1, &id);
  glBindBuffer(gl_buffer_type, id);
  glBufferData(gl_buffer_type, allocated, nullptr, GL_STATIC_DRAW); // GL_STREAM_DRAW

#ifndef NDEBUG //--контроль создания буфера--------------------------------
  GLint d_size = 0;
  glGetBufferParameteriv(gl_buffer_type, GL_BUFFER_SIZE, &d_size);
  assert(allocated == d_size);
#endif //------------------------------------------------------------------

  glBindBuffer(gl_buffer_type, 0);
}


///
/// \brief vbo::allocate
/// \param al
/// \param data
///
/// \details Cоздание графического буфера и заполнение его данными
///
void vbo_base::allocate(GLsizeiptr al, const GLvoid* data)
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
void vbo_base::attrib(GLuint index, GLint d_size, GLenum type, GLboolean normalized,
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
void vbo_base::attrib_i(GLuint index, GLint d_size, GLenum type,
  GLsizei stride, const GLvoid* pointer)
{
  glEnableVertexAttribArray(index);
  glBindBuffer(gl_buffer_type, id);
  glVertexAttribIPointer(index, d_size, type, stride, pointer);
  glBindBuffer(gl_buffer_type, 0);
}


///
/// \brief vbo_ext::vbo_ext
/// \param type
///
vbo_ext::vbo_ext(GLenum type): vbo_base(type)
{
  glGenBuffers(1, &id_subbuf);
  glBindBuffer(GL_COPY_WRITE_BUFFER, id_subbuf);
  glBufferData(GL_COPY_WRITE_BUFFER, bytes_per_side, nullptr, GL_STATIC_DRAW);
  glBindBuffer(GL_COPY_WRITE_BUFFER, 0);
}


///
/// \brief vbo_ext::remove
/// \param dest
/// \return
/// \details Для удаления данных из VBO используется перемещение: на место
/// удаляемого блока перемещаются данные из конца буфера, заменяя их. Длина
/// активной части буфера уменьшается на размер удаленного блока.
///
/// Возвращается значение границы VBO после переноса данных. Она равна
/// предыдущему значению адреса блока данных, которые были перемещены.
///
GLsizeiptr vbo_ext::remove(GLsizeiptr dest, GLsizeiptr data_size)
{
  if(data_size >= hem) hem = 0;
  if(hem == 0) return hem;

#ifndef NDEBUG
  if((dest + data_size) > hem)
    std::printf("vbo_ext::remove error: dest %li + data_size %li  > hem %li\n",
                static_cast<long>(dest), static_cast<long>(data_size), static_cast<long>(hem));
#endif
  auto src = hem - data_size; // Адрес крайнего на хвосте блока данных, которые будут перемещены.

  if(src == dest)
  {             // Если удаляемый блок оказался в конце буфера, то только
    hem = src;  // сдвигаем гранцу зоны на размер блока (без перемещения данных).
  }
  else
  {
    glBindBuffer(gl_buffer_type, id);
    glCopyBufferSubData(gl_buffer_type, gl_buffer_type, src, dest, data_size);
    glBindBuffer(gl_buffer_type, 0);
    hem = src;
  }
  return hem;
}


///
/// \brief vbo::append
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
GLsizeiptr vbo_ext::append(const GLvoid* data, GLsizeiptr data_size)
{
  #ifndef NDEBUG // проверка свободного места в буфере----------------------
  if((allocated - hem) < data_size) ERR("VBO::SubDataAppend got overflow buffer");
  if(data == nullptr) ERR("VBO::SubDataAppend nullptr");
  #endif //------------------------------------------------------------------

  glBindBuffer(gl_buffer_type, id);
  glBufferSubData(gl_buffer_type, hem, data_size, data);
  glBindBuffer(gl_buffer_type, 0);
  GLsizeiptr res = hem;
  hem += data_size;
  return res;
}


///
/// \brief vbo_ext::data_get
/// \param offset
/// \param size
/// \param data
/// \details Если читать из рендер-буфера напрямую в ОЗУ, то после считывания
/// скорость операции перемещения данных внутри памяти GPU устанавливается
/// равной скорости обмена с CPU. Чтобы это обойти, создается промежуточный
/// буфер (id_subbuf), через который и производится чтение данных.
///
void vbo_ext::data_get(GLintptr offset, GLsizeiptr sz, GLvoid* dst)
{
#ifndef NDEBUG
  if(sz > bytes_per_side) ERR("vbo_ext::data_get > bytes_per_snip");
#endif

  glBindBuffer(GL_COPY_WRITE_BUFFER, id_subbuf);
  glBindBuffer(GL_COPY_READ_BUFFER, id);
  glCopyBufferSubData(GL_COPY_READ_BUFFER, GL_COPY_WRITE_BUFFER, offset, 0, sz);
  glBindBuffer(GL_COPY_READ_BUFFER, 0);
  glBindBuffer(GL_COPY_WRITE_BUFFER, 0);

  glBindBuffer(GL_COPY_READ_BUFFER, id_subbuf);
  //glGetBufferSubData(GL_COPY_WRITE_BUFFER, offset, sz, dst);
  GLvoid* ptr = glMapBufferRange(GL_COPY_READ_BUFFER, 0, sz, GL_MAP_READ_BIT);
  memcpy(dst, ptr, sz);
  glUnmapBuffer(GL_COPY_READ_BUFFER);
  glBindBuffer(GL_COPY_READ_BUFFER, 0);

  if(glGetError() != GL_NO_ERROR) info("err in vbo_ext::data_get");
}


/*
///
/// \brief vbo::data_update
/// \param d_size
/// \param data
/// \param dst
/// \return
/// \details Замена блока данных в указанном месте
///
void vbo_ext::data_update(GLsizeiptr dist, const GLvoid* data, GLsizeiptr data_size)
{
#ifndef NDEBUG
    if(dist > (hem - data_size)) ERR("ERR vbo::data_update");
#endif
    glBindBuffer(gl_buffer_type, id);
    glBufferSubData(gl_buffer_type, dist, data_size, data);
    glBindBuffer(gl_buffer_type, 0);
  }
*/

} //tr
