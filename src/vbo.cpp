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
  /// \details Замена блока данных в указанном месте
  ///
  void vbo::data_update(GLsizeiptr d_size, const GLvoid* data, GLsizeiptr dst)
  {
#ifndef NDEBUG
    if(dst > (hem - d_size)) ERR("ERR vbo::data_update"); // протухший адрес кэша
#endif
    if(GL_ARRAY_BUFFER == gl_buffer_type) glBindBuffer(GL_ARRAY_BUFFER, id);
    glBufferSubData(gl_buffer_type, dst, d_size, data);
    if(GL_ARRAY_BUFFER == gl_buffer_type) glBindBuffer(GL_ARRAY_BUFFER, 0);
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
  /// \brief snip::vbo_jam
  /// \param VBOdata
  /// \param dst
  ///
  /// \details Перемещение блока данных из конца ближе к началу VBO буфера
  ///
  /// Эта функция используется только для крайних блоков данных, расположеных
  /// в конце VBO. Данные перемещаются на указанное место (dst) ближе к началу
  /// буфера, после чего активная граница VBO сдвигается к началу на размер
  /// перемещеного блока данных.
  ///
  void vbo::vbo_jam(snip* S, GLintptr dst)
  {
    jam_data(S->data_offset, dst, bytes_per_snip);
    S->data_offset = dst;
  }


  ///
  /// \brief rdb::put_in_vbo
  /// \param Rig
  /// \param Point
  ///
  void vbo::data_place(snip& Snip, const f3d& Point)
  {

    if(CachedOffset.empty()) // Если кэш пустой, то добавляем данные в конец VBO
    {
      Snip.data_offset = append(Snip.data, Point);
      render_points += indices_per_snip;      // увеличить число точек рендера
    }
    else // если в кэше есть адреса свободных мест, то используем их
    {
      Snip.data_offset = CachedOffset.front();
      update(Snip.data, Point, CachedOffset.front());
      CachedOffset.pop_front(); // укоротить кэш
    }
    VisibleSnips[Snip.data_offset] = &Snip; // добавить ссылку
  }


  ///
  /// \brief rdb::side_remove
  /// \param Side
  ///
  void vbo::data_remove(std::vector<snip>& Side)
  {
    if(Side.empty()) return;

    for(auto& Snip: Side)
    {
      VisibleSnips.erase(Snip.data_offset);
      CachedOffset.push_front(Snip.data_offset);
    }
  }


  ///
  /// \brief rdb::clear_cashed_snips
  /// \details Удаление элементов по адресам с кэше и сжатие данных в VBO
  ///
  /// Если в кэше есть адрес блока из середины VBO, то в него переносим данные
  /// из конца VBO и сжимаем буфер на длину одного блока. Если адрес из кэша
  /// указывает на крайний блок в конце VBO, то сжимаем буфер сдвигая границу
  /// на длину одного блока.
  ///
  /// Не забываем уменьшить число элементов в рендере.
  ///
  void vbo::clear_cashed_snips(void)
  {
    if(CachedOffset.empty()) return;

    // Выбрать самый крайний элемент VBO на границе блока данных
    GLsizeiptr data_src = get_hem();

    if(0 == render_points)
    {
      CachedOffset.clear();   // очистить все, сообщить о проблеме
      VisibleSnips.clear();   // и закончить обработку кэша
      return;
    }

    #ifndef NDEBUG
    if (data_src == 0 ) {      // Если (вдруг!) данных нет, то
      CachedOffset.clear();   // очистить все, сообщить о проблеме
      VisibleSnips.clear();   // и закончить обработку кэша
      render_points = 0;
      info("WARNING: space::clear_cashed_snips got empty data_src\n");
      return;
    }

    /// Граница буфера (VBOdata.get_hem()), если она не равна нулю,
    /// не может быть меньше размера блока данных (bytes_per_snip)
    if(data_src < bytes_per_snip)
      ERR ("BUG!!! space::jam_vbo got error address in VBO");
    #endif

    data_src -= bytes_per_snip; // адрес последнего блока

    /// Если крайний блок не в списке VisibleSnips, то он и не в рендере.
    /// Поэтому просто отбросим его, сдвинув границу буфера VBO. Кэш не
    /// изменяем, так как в контейнере "forward_list" удаление элементов
    /// из середины списка - затратная операция.
    ///
    /// Внимание! Так как после этого где-то в кэше остается невалидный
    /// (за рабочей границей VBO) адрес блока, то при использовании
    /// адресов из кэша надо делать проверку - не "протухли" ли они.
    ///
    if(VisibleSnips.find(data_src) == VisibleSnips.end())
    {
      shrink(bytes_per_snip);    // укоротить VBO данных
      render_points -= indices_per_snip; // уменьшить число точек рендера
      return;                                // и прервать обработку кэша
    }

    GLsizeiptr data_dst = data_src;
    // Извлечь из кэша адрес
    while(data_dst >= data_src)
    {
      if(CachedOffset.empty()) return;
      data_dst = CachedOffset.front();
      CachedOffset.pop_front();

      // Идеальный вариант, когда освободившийся блок оказался крайним в VBO
      if(data_dst == data_src)
      {
        shrink(bytes_per_snip);    // укоротить VBO данных
        render_points -= indices_per_snip; // уменьшить число точек рендера
        return;                                // закончить шаг обработки
      }
    }

    // Самый частый и самый сложный вариант
    try { // Если есть отображаемый data_src и меньший data_dst из кэша, то
      snip* Snip = VisibleSnips.at(data_src);  // найти перемещаемый снип,
      VisibleSnips.erase(data_src);            // удалить ссылку с карты
      vbo_jam(Snip, data_dst);                 // переместить данные в VBO
      VisibleSnips[data_dst] = Snip;           // внести новую ссылку в карту
      render_points -= indices_per_snip;       // Так как данные из хвоста - подрезать длину
    } catch(std::exception & e) {
      ERR(e.what());
    } catch(...) {
      ERR("rigs::clear_cashed_snips got error VisibleSnips[data_src]");
    }
  }

} //tr
