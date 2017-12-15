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
  vertex::vertex(GLfloat *data)
  {
    position.x = data++;
    position.y = data++;
    position.z = data++;
    position.w = data++;
    color.r    = data++;
    color.g    = data++;
    color.b    = data++;
    color.a    = data++;
    normal.x   = data++;
    normal.y   = data++;
    normal.z   = data++;
    normal.w   = data++;
    fragment.u = data++;
    fragment.v = data++;
    return;
  }

  //## конструктор по-умолчанию
  snip::snip(const tr::f3d & p)
  {
    relocate(p);
    if( .0f == p.x && .0f == p.z )
    {
      for(size_t i = 0; i < 4; i++)
        *vertices[i].fragment.v += .375f;
    }
  }

  //## сдвиг прямоугольника
  void snip::relocate(const tr::f3d & point)
  {
    *vertices[0].position.x += point.x;
    *vertices[1].position.x += point.x;
    *vertices[2].position.x += point.x;
    *vertices[3].position.x += point.x;

    *vertices[0].position.y += point.y;
    *vertices[1].position.y += point.y;
    *vertices[2].position.y += point.y;
    *vertices[3].position.y += point.y;

    *vertices[0].position.z += point.z;
    *vertices[1].position.z += point.z;
    *vertices[2].position.z += point.z;
    *vertices[3].position.z += point.z;

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
