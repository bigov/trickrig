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


///
/// \brief snip::operator =
/// \param Other
/// \return
///
  snip& snip::operator= (const snip& Other)
  {
    if(this != &Other) copy_data(Other);
    return *this;
  }

  ///
  /// \brief snip::snip
  /// \param Other
  /// \details ## дублирующий конструктор
  ///
  snip::snip(const snip& Other)
  {
    copy_data(Other);
  }

  ///
  /// \brief snip::copy_data
  /// \param Other
  /// \details ## Копирование данных из другого снипа
  ///
  void snip::copy_data(const snip & Other)
  {
    //data_offset = Other.data_offset;
    memcpy(data, Other.data, bytes_per_snip);
  }


  ///
  /// \brief   настройка текстуры
  /// \param u - номер по-горизонтали
  /// \param v - номер по-вертикали
  /// \details Устанавливает на Снип текстуру из ячейки [u,v]
  ///
  void snip::texture_set(GLfloat u, GLfloat v)
  {
    size_t n = 0; // номер вершины
    data[ROW_SIZE * n + U] = u * u_size;
    data[ROW_SIZE * n + V] = v * v_size;

    n += 1;
    data[ROW_SIZE * n + U] = (u + 1) * u_size;
    data[ROW_SIZE * n + V] = v * v_size;

    n += 1;
    data[ROW_SIZE * n + U] = (u + 1) * u_size;
    data[ROW_SIZE * n + V] = (v + 1) * v_size;

    n += 1;
    data[ROW_SIZE * n + U] = u * u_size;
    data[ROW_SIZE * n + V] = (v + 1) * v_size;
  }


  ///
  /// \brief      настройка текстуры
  /// \param u  - номер текстуры по-горизонтали
  /// \param v  - номер текстуры по-вертикали
  /// \param Dс - смещение для всех четырех точек в нормированном диапазоне [0.0; 1.0]
  /// \details Устанавливает на Снип фрагмент текстуры с обрезкой.
  ///
  void snip::texture_fragment(GLfloat u, GLfloat v, const std::array<float, 8>& Dc)
  {
    texture_set(u, v);
    auto D = Dc;

    D[0] =  D[0] * u_size;
    D[1] =  (1.f - D[1]) * v_size;

    D[2] = -(1.f - D[2]) * u_size;
    D[3] =  (1.f - D[3]) * v_size;

    D[4] = -(1.f - D[4]) * u_size;
    D[5] = -D[5] * v_size;

    D[6] = -D[6] * u_size;
    D[7] = -D[7] * v_size;

    for(size_t n = 0; n < 4; ++n)
    {
      data[ROW_SIZE * n + U] += D[n * 2 ];
      data[ROW_SIZE * n + V] += D[n * 2 + 1];
    }
  }


  ///
  /// \brief snip::vertex_coord Возвращает координаты вершины № idx
  /// \param idx
  /// \return
  ///
  glm::vec4 snip::vertex_coord(size_t idx)
  {
    return glm::vec4 { data[ROW_SIZE * idx + X],
                       data[ROW_SIZE * idx + Y],
                       data[ROW_SIZE * idx + Z],
                       1.0 };
  }


  ///
  /// \brief shift
  /// \param V
  ///
  void snip::shift(const glm::vec3 &V)
  {
    for(size_t n = 0; n < 4; ++n)
    {
      data[ROW_SIZE * n + X] += V.x;
      data[ROW_SIZE * n + Y] += V.y;
      data[ROW_SIZE * n + Z] += V.z;
    }
  }


  ///
  /// \brief snip::flip_y
  ///
  void snip::flip_y(void)
  {
    GLfloat tmp[digits_per_snip] = {0.0f};
    memcpy(tmp, data, bytes_per_snip);

    memcpy(&data[0 * digits_per_vertex], &tmp[2 * digits_per_vertex], 4 * sizeof(GLfloat));
    memcpy(&data[1 * digits_per_vertex], &tmp[3 * digits_per_vertex], 4 * sizeof(GLfloat));
    memcpy(&data[2 * digits_per_vertex], &tmp[0 * digits_per_vertex], 4 * sizeof(GLfloat));
    memcpy(&data[3 * digits_per_vertex], &tmp[1 * digits_per_vertex], 4 * sizeof(GLfloat));
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
  void snip::vbo_append(const f3d& Point, vbo& VBOdata)
  {
    GLfloat cache[digits_per_snip] = {0.0f};
    memcpy(cache, data, bytes_per_snip);

    for(size_t n = 0; n < vertices_per_snip; n++)
    {
      cache[ROW_SIZE * n + X] += Point.x;
      cache[ROW_SIZE * n + Y] += Point.y;
      cache[ROW_SIZE * n + Z] += Point.z;
    }

    data_offset = VBOdata.data_append( bytes_per_snip, cache );
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
  bool snip::vbo_update(const f3d &Point, vbo & VBOdata, GLsizeiptr dst)
  {

    GLfloat vbo_data[tr::digits_per_snip] = {0.0f};
    memcpy(vbo_data, data, tr::bytes_per_snip);
    for(size_t n = 0; n < tr::vertices_per_snip; n++)
    {
      vbo_data[ROW_SIZE * n + X] += Point.x;
      vbo_data[ROW_SIZE * n + Y] += Point.y;
      vbo_data[ROW_SIZE * n + Z] += Point.z;
    }

    if(VBOdata.data_update( tr::bytes_per_snip, vbo_data, dst ))
    {
      data_offset = dst;
      return true;
    }
    return false;
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
  void snip::vbo_jam(vbo &VBOdata, GLintptr dst)
  {
    VBOdata.jam_data(data_offset, dst, tr::bytes_per_snip);
    data_offset = dst;
  }

} //namespace
