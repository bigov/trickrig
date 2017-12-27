//============================================================================
//
// file: snip.cpp
//
// Элементы поверхности в риге
//
//============================================================================

#include "snip.hpp"

namespace tr
{
  snip::snip(void)
  {
    return;
  }

  //## конструктор по-умолчанию
  snip::snip(const tr::f3d & p)
  {
    point_set(p);
    return;
  }

  tr::snip& snip::operator= (const snip & Other)
  {
    if(this != &Other) copy_data(Other);
    return *this;
  }

  //## дублирующий конструктор
  snip::snip(const snip & Other)
  {
    copy_data(Other);
    return;
  }

  //## Копирование данных из другого снипа
  void snip::copy_data(const snip & Other)
  {
    data_offset = Other.data_offset;

    for(size_t n = 0; n < tr::digits_per_snip; n++)
      data[n] = Other.data[n];

    for(size_t n = 0; n < tr::indices_per_snip; n++)
      idx[n] = Other.idx[n];

    return;
  }

  //## настройка текстуры
  void snip::texture_set(GLfloat u, GLfloat v)
  {
    for(size_t n = 0; n < tr::vertices_per_snip; n++)
    {
      data[ROW_STRIDE * n + FRAGM_U] += u;
      data[ROW_STRIDE * n + FRAGM_V] += v;
    }
    return;
  }

  //## Перенос снипа на указанную точку в пространстве
  void snip::point_set(const tr::f3d &P)
  {
    size_t n = 0;
    // В снипе 4 вершины. Найти индекс опорной (по минимальному значению)
    for(size_t i = 1; i < 4; i++)
      if((
           data[n * ROW_STRIDE + COORD_X]
         + data[n * ROW_STRIDE + COORD_Y]
         + data[n * ROW_STRIDE + COORD_Z]
            ) > (
           data[i * ROW_STRIDE + COORD_X]
         + data[i * ROW_STRIDE + COORD_Y]
         + data[i * ROW_STRIDE + COORD_Z]
         )) n = i;

    // вычислить дистанцию смещения опорной точки
    tr::f3d d = { P.x - floor(data[n * ROW_STRIDE + COORD_X]),
                  P.y - floor(data[n * ROW_STRIDE + COORD_Y]),
                  P.z - floor(data[n * ROW_STRIDE + COORD_Z])};

    for(size_t n = 0; n < tr::vertices_per_snip; n++)
    {
      data[ROW_STRIDE * n + COORD_X] += d.x;
      data[ROW_STRIDE * n + COORD_Y] += d.y;
      data[ROW_STRIDE * n + COORD_Z] += d.z;
    }
    return;
  }

  //## сдвиг индексов на опорную точку
  GLsizei *snip::reindex(GLsizei stride)
  {
    stride -= idx[0]; // для возврата индексов в исходное положение
    for(size_t i = 0; i < tr::indices_per_snip; i++) idx[i] += stride;
    return idx;
  }

  //## добавление данных в буферы VBO
  void snip::vbo_append(tr::vbo &VBOdata, tr::vbo &VBOidx)
  {
  /* Добавляет данные в конец VBO буфера данных и VBO буфера индексов
   * и запоминает смещение адресов в VBO где данные были записаны
   */
    data_offset = VBOdata.data_append( tr::snip_data_bytes, data );
    VBOidx.data_append( tr::snip_index_bytes, reindex( data_offset/tr::snip_bytes_per_vertex ));
    return;
  }

  //## обновление данных в VBO буфере данных и VBO буфере индексов
  bool snip::vbo_update(tr::vbo &VBOdata, tr::vbo &VBOidx, GLsizeiptr offset)
  {
    /**
     * Целевой адрес для перемещения блока данных в VBO (параметр "offset") берется
     * обычно из кэша. При этом может возникнуть ситуация, когда в кэше остаются
     * адреса блоков за текущей границей VBO. Такой адрес считается "протухшим",
     * блок данных не перемещается, функция возвращает false.
     */
    data_offset = offset;
    GLsizeiptr idx_offset = (offset / tr::digits_per_snip) * tr::indices_per_snip;

    if(!VBOdata.data_update( tr::snip_data_bytes, data, data_offset ))
      return false;

    // Если блок данных был успешно перенесен, а при попытке перемещения
    // индекса возникла ошибка, то значит что-то пошло не так.
    if(!VBOidx.data_update( tr::snip_index_bytes,
      reindex( data_offset / tr::snip_bytes_per_vertex ), idx_offset ))
      ERR("snip::vbo_update can't update index VBO.");

    return true;
  }

  //## перемещение блока данных внутри VBO буфера
  void snip::vbo_jam(tr::vbo &VBOdata, tr::vbo &VBOidx, GLintptr dst)
  {
    VBOdata.jam_data(data_offset, dst, tr::snip_data_bytes);
    GLintptr idx_src = (data_offset / tr::digits_per_snip) * tr::indices_per_snip;
    GLintptr idx_dst = (dst / tr::digits_per_snip) * tr::indices_per_snip;
    VBOidx.jam_data(idx_src, idx_dst, tr::snip_index_bytes);
    data_offset = dst;
    return;
  }

} //namespace
