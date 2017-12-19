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
    return;
  }

  //## конструктор по-умолчанию
  snip::snip(const tr::f3d & p)
  {
    v_reset();
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

    v_reset(); // настроить вспомогательные указатели

    return;
  }

  //## Настройка вспомогательных указателей
  void snip::v_reset()
  {
  /* V - это вспомогательная структура именованых указателей
   * на данные вершин в массиве. Используется только для создания
   * более наглядного исходного кода программы и уменьшения
   * числа "магических" индексов/чисел.
   *
   * Так как при копировании данных между снипами данные указателей
   * нового снипа будут ссылаться на адреса данных снипа-источника,
   * то после копирования необходимо каждый раз перенастраивать
   * указатели на актуальные адреса данных в "своем" снипе.
   *
   * При создании шаблонного снипа с данными по умолчанию так же
   * необходимо каждый раз вызывать процедуру настройки.
   */
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

  //## Перенос снипа на указанную точку в пространстве
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
