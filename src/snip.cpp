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
                       1.0f };
  }


  ///
  /// \brief snip::normal_coord
  /// \param idx
  /// \return
  ///
  glm::vec4 snip::vertex_normal(size_t idx)
  {
    return glm::vec4 { data[ROW_SIZE * idx + NX],
                       data[ROW_SIZE * idx + NY],
                       data[ROW_SIZE * idx + NZ],
                       0.0f };
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

} //namespace
