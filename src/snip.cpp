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
    v_reset();
  }

  //## конструктор по-умолчанию
  snip::snip(const tr::f3d & p)
  {
    v_reset();
    point_set(p);
  }

  //## дублирующий конструктор
  snip::snip(const snip & Other)
  {
    data_offset = Other.data_offset;

    for(size_t n = 0; n < tr::digits_per_snip; n++)
      data[n] = Other.data[n];

    for(size_t n = 0; n < tr::indices_per_snip; n++)
      idx[n] = Other.idx[n];

    v_reset(); // переписать адреса ссылок на данные

    return;
  }

  //## Настройка адресов указателей
  void snip::v_reset()
  {
    size_t i = 0;
    for(size_t n = 0; n < tr::vertices_per_snip; n++)
    {
      V[n].point.x  = &data[i++];
      V[n].point.y  = &data[i++];
      V[n].point.z  = &data[i++];
      V[n].point.w  = &data[i++];
      V[n].color.r  = &data[i++];
      V[n].color.g  = &data[i++];
      V[n].color.b  = &data[i++];
      V[n].color.a  = &data[i++];
      V[n].normal.x = &data[i++];
      V[n].normal.y = &data[i++];
      V[n].normal.z = &data[i++];
      V[n].normal.w = &data[i++];
      V[n].fragm.u  = &data[i++];
      V[n].fragm.v  = &data[i++];
    }
    return;
  }

  //## настройка текстуры
  void snip::texture_set(GLfloat u, GLfloat v)
  {
    for(size_t n = 0; n < tr::vertices_per_snip; n++)
    {
      *V[n].fragm.u += u; *V[n].fragm.v += v;
    }
    return;
  }

  //## установка четырехугольника по координатам
  void snip::point_set(const tr::f3d &P)
  {
    for(size_t n = 0; n < tr::vertices_per_snip; n++)
    {
      *V[n].point.x += P.x;
      *V[n].point.y += P.y;
      *V[n].point.z += P.z;
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
  void snip::vbo_update(tr::vbo &VBOdata, tr::vbo &VBOidx, GLsizeiptr offset)
  {
    data_offset = offset;
    GLsizeiptr idx_offset = (offset / tr::digits_per_snip) * tr::indices_per_snip;

    VBOdata.data_update( tr::snip_data_bytes, data, data_offset );
    VBOidx.data_update( tr::snip_index_bytes,
      reindex( data_offset / tr::snip_bytes_per_vertex ), idx_offset );

    return;
  }

  //## перемещение блока данных внутри VBO буфера
  void snip::jam_data(tr::vbo &VBOdata, tr::vbo &VBOidx, GLintptr dst)
  {
    VBOdata.jam_data(data_offset, dst, tr::snip_data_bytes);
    GLintptr idx_src = (data_offset / tr::digits_per_snip) * tr::indices_per_snip;
    GLintptr idx_dst = (dst / tr::digits_per_snip) * tr::indices_per_snip;
    VBOidx.jam_data(idx_src, idx_dst, tr::snip_index_bytes);
    data_offset = dst;
    return;
  }

} //namespace
