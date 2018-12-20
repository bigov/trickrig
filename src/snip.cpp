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
  snip& snip::operator= (const snip &Other)
  {
    if(this != &Other) copy_data(Other);
    return *this;
  }

  //## дублирующий конструктор
  snip::snip(const snip & Other)
  {
    copy_data(Other);
  }

  //## Копирование данных из другого снипа
  void snip::copy_data(const snip & Other)
  {
    data_offset = Other.data_offset;

    for(size_t n = 0; n < tr::digits_per_snip; n++)
      data[n] = Other.data[n];
  }

  ///
  /// \brief   настройка текстуры
  /// \param u - номер по-горизонтали
  /// \param v - номер по-вертикали
  ///
  void snip::texture_set(u_char u, u_char v)
  {
    GLfloat u_size = 0.125f;
    GLfloat v_size = 0.125f;

    for(size_t n = 0; n < tr::vertices_per_snip; n++)
    {
      data[SNIP_ROW_DIGITS * n + SNIP_U] = u * u_size;
      data[SNIP_ROW_DIGITS * n + SNIP_V] = v * v_size;
    }
  }

  ///
  /// \brief snip::vertex_coord Возвращает координаты вершины № idx
  /// \param idx
  /// \return
  ///
  glm::vec4 snip::vertex_coord(size_t idx)
  {
    return glm::vec4 { data[SNIP_ROW_DIGITS * idx + SNIP_X],
                       data[SNIP_ROW_DIGITS * idx + SNIP_Y],
                       data[SNIP_ROW_DIGITS * idx + SNIP_Z],
                       1.0 };
  }

  ///
  /// \brief
  /// Добавляет свои данные в конец буфера данных VBO и запоминает свой адрес
  /// смещения.
  ///
  /// \details
  /// Координаты вершин снипов хранятся в нормализованом виде, поэтому перед
  /// отправкой в VBO все данные снипа копируются во временный кэш, где
  /// координаты вершин пересчитываются с учетом координат (TODO: сдвига и
  /// поворота рига-контейнера) и преобразованные данные записываются в VBO.
  ///
  void snip::vbo_append(const f3d &Point, vbo & VBOdata)
  {
    GLfloat cache[tr::digits_per_snip] = {0.0f};
    memcpy(cache, data, tr::bytes_per_snip);
    for(size_t n = 0; n < tr::vertices_per_snip; n++)
    {
      cache[SNIP_ROW_DIGITS * n + SNIP_X] += Point.x;
      cache[SNIP_ROW_DIGITS * n + SNIP_Y] += Point.y;
      cache[SNIP_ROW_DIGITS * n + SNIP_Z] += Point.z;
    }

    data_offset = VBOdata.data_append( tr::bytes_per_snip, cache );
  }

  //## обновление данных в VBO буфере
  bool snip::vbo_update(const f3d &Point, vbo & VBOdata, GLsizeiptr dst)
  {
  /// Целевой адрес для перемещения блока данных в VBO (параметр "offset")
  /// берется обычно из кэша. При этом может возникнуть ситуация, когда в кэше
  /// остаются адреса блоков за текущей границей VBO. Такой адрес считается
  /// "протухшим", блок данных не перемещается, функция возвращает false.
  ///
  /// Координаты вершин снипов в трике хранятся в нормализованом виде,
  /// поэтому перед отправкой данных в VBO координаты вершин пересчитываются
  /// в соответствии с координатами и данными(shift) связаного рига,

    GLfloat vbo_data[tr::digits_per_snip] = {0.0f};
    memcpy(vbo_data, data, tr::bytes_per_snip);
    for(size_t n = 0; n < tr::vertices_per_snip; n++)
    {
      vbo_data[SNIP_ROW_DIGITS * n + SNIP_X] += Point.x;
      vbo_data[SNIP_ROW_DIGITS * n + SNIP_Y] += Point.y;
      vbo_data[SNIP_ROW_DIGITS * n + SNIP_Z] += Point.z;
    }

    if(VBOdata.data_update( tr::bytes_per_snip, vbo_data, dst ))
    {
      data_offset = dst;
      return true;
    }
    return false;
  }

  //## Перемещение блока данных из конца ближе к началу VBO буфера
  void snip::vbo_jam(vbo &VBOdata, GLintptr dst)
  {
  /// Эта функция используется только для крайних блоков данных, расположеных
  /// в конце VBO. Данные перемещаются на указанное место (dst) ближе к началу
  /// буфера, после чего активная граница VBO сдвигается к началу на размер
  /// перемещеного блока данных.

    VBOdata.jam_data(data_offset, dst, tr::bytes_per_snip);
    data_offset = dst;
  }

} //namespace
