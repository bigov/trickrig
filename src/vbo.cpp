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
void vbo_mem::shrink(GLsizeiptr delta)
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
void vbo_base::allocate(GLsizeiptr al)
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
/// \brief vbo::jam_data
/// \param src
/// \param dst
/// \param d_size
///
/// \details Сжатие буфера VBO за счет перемещения блока данных из конца
/// на свободное место ближе к началу буфера. Адрес начала свободного блока
/// берется из кэша.
///
/// После перемещения активная граница VBO сдвигается к началу на размер
/// перемещеного блока данных.
///
void vbo_mem::jam_data(GLintptr src, GLintptr dst, GLsizeiptr data_size)
{
  #ifndef NDEBUG // контроль направления переноса - только крайний в конце блок
    if((src + data_size) != hem) ERR("vbo::jam_data got err src address");
    if(dst > (src - data_size))  ERR("vbo::jam_data: dst + size > src");
  #endif

  glBindBuffer(gl_buffer_type, id);
  glCopyBufferSubData(gl_buffer_type, gl_buffer_type, src, dst, data_size);
  glBindBuffer(gl_buffer_type, 0);
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
GLsizeiptr vbo_mem::data_append(GLsizeiptr d_size, const GLvoid* data)
{
  #ifndef NDEBUG // проверка свободного места в буфере----------------------
  if((allocated - hem) < d_size) ERR("VBO::SubDataAppend got overflow buffer");
  #endif //------------------------------------------------------------------

  glBindBuffer(gl_buffer_type, id);
  glBufferSubData(GL_ARRAY_BUFFER, hem, d_size, data);
  glBindBuffer(gl_buffer_type, 0);
  GLsizeiptr res = hem;
  hem += d_size;

  return res;
}


///
/// \brief vbo::data_update
/// \param d_size
/// \param data
/// \param dst
/// \return
/// \details Замена блока данных в указанном месте
///
void vbo_mem::data_update(GLsizeiptr d_size, const GLvoid* data, GLsizeiptr dst)
{
#ifndef NDEBUG
    if(dst > (hem - d_size)) ERR("ERR vbo::data_update"); // протухший адрес кэша
#endif
    glBindBuffer(gl_buffer_type, id);
    glBufferSubData(gl_buffer_type, dst, d_size, data);
    glBindBuffer(gl_buffer_type, 0);
  }


///
/// \brief
/// Добавляет данные в конец буфера данных VBO и фиксирует адрес смещения.
///
/// \details
/// Координаты вершин снипов хранятся в нормализованом виде, поэтому перед
/// отправкой в VBO все данные снипа копируются во временный кэш, где
/// координаты вершин пересчитываются с учетом координат (TODO: сдвига и
/// поворота рига-контейнера) и преобразованные данные записываются в VBO.
///
GLsizeiptr vbo::append(GLfloat* s_data, const f3d& Point)
{
  GLfloat cache[digits_per_snip] = {0.0f};
  memcpy(cache, s_data, bytes_per_snip);

  for(size_t n = 0; n < vertices_per_snip; n++)
  {
    cache[ROW_SIZE * n + X] += Point.x;
    cache[ROW_SIZE * n + Y] += Point.y;
    cache[ROW_SIZE * n + Z] += Point.z;
  }

  return data_append( bytes_per_snip, cache );
}


///
/// \brief snip::vbo_update
/// \param Point
/// \param VBOdata
/// \param dst
/// \return
///
/// \details обновление данных в VBO буфере
///
/// Целевой адрес для перемещения блока данных в VBO (параметр "offset")
/// берется обычно из кэша. При этом может возникнуть ситуация, когда в кэше
/// остаются адреса блоков за текущей границей VBO. Такой адрес считается
/// "протухшим", блок данных не перемещается, функция возвращает false.
///
/// Координаты вершин снипов в трике хранятся в нормализованом виде,
/// поэтому перед отправкой данных в VBO координаты вершин пересчитываются
/// в соответствии с координатами и данными(shift) связаного рига,
///
void vbo::update(GLfloat* s_data, const f3d &Point, GLsizeiptr dst)
{
  GLfloat new_data[digits_per_snip] = {0.0f};
  memcpy(new_data, s_data, bytes_per_snip);
  for(size_t n = 0; n < vertices_per_snip; n++)
  {
    new_data[ROW_SIZE * n + X] += Point.x;
    new_data[ROW_SIZE * n + Y] += Point.y;
    new_data[ROW_SIZE * n + Z] += Point.z;
  }
  data_update( bytes_per_snip, new_data, dst );
}


///
/// \brief vbo::data_place
/// \param Rig
/// \param Point
///
void vbo::data_place(snip& Snip, const f3d& Point)
{
  Snip.data_offset = append(Snip.data, Point);
  render_points += indices_per_snip;      // увеличить число точек рендера
  VisibleSnips[Snip.data_offset] = &Snip; // добавить ссылку
}


///
/// \brief vbo::data_remove
/// \param Side
///
/// \details Удаление данных из VBO
///
void vbo::data_remove(std::vector<snip>& Side)
{
  if(Side.empty()) return;

  for(auto& Snip: Side)
  {
    VisibleSnips.erase(Snip.data_offset);
  }
}


} //tr
