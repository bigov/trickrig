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

  //## добавление данных в буферы VBO
  void snip::vbo_append(tr::vbo & VBOdata)
  {
  /// Добавляет данные в конец VBO буфера данных и запоминает смещение
  /// адреса где в VBO были записаны данные

    data_offset = VBOdata.data_append( tr::bytes_per_snip, data );
    return;
  }

  //## обновление данных в VBO буфере
  bool snip::vbo_update(tr::vbo & VBOdata, GLsizeiptr dst)
  {
  /// Целевой адрес для перемещения блока данных в VBO (параметр "offset")
  /// берется обычно из кэша. При этом может возникнуть ситуация, когда в кэше
  /// остаются адреса блоков за текущей границей VBO. Такой адрес считается
  /// "протухшим", блок данных не перемещается, функция возвращает false.

    if(VBOdata.data_update( tr::bytes_per_snip, data, dst ))
    {
      data_offset = dst;
      return true;
    }
    return false;
  }

  //## Перемещение блока данных из конца ближе к началу VBO буфера
  void snip::vbo_jam(tr::vbo & VBOdata, GLintptr dst)
  {
  /// Эта функция используется только для крайних блоков данных, расположеных
  /// в конце VBO. Данные перемещаются на указанное место (dst) ближе к началу
  /// буфера, после чего активная граница VBO сдвигается к началу на размер
  /// перемещеного блока данных.

    VBOdata.jam_data(data_offset, dst, tr::bytes_per_snip);
    data_offset = dst;
    return;
  }

} //namespace
