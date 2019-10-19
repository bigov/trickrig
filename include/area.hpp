/**
 *
 * file: area.hpp
 *
 * Заголовок класса управления элементами области 3D пространства
 *
 */

#ifndef AREA_HPP
#define AREA_HPP

#include <queue>
#include "glsl.hpp"
#include "voxbuf.hpp"
#include "vbo.hpp"


namespace tr
{
///
/// \brief class area
/// \details Управление картой воксов
class area
{
  private:
    std::unique_ptr<vox_buffer> VoxBuffer = nullptr;
    std::queue<i3d> QueueLoad {}; // адреса загружаемых воксов
    std::queue<i3d> QueueWipe {}; // адреса выгружаемах воксов

    int vox_side_len = 0;   // Длина стороны вокса
    int lod_dist_far = 0;   // Расстояние от камеры до внешней границы LOD, кратное размеру вокса
    i3d Location {0, 0, 0}; // Origin вокса, над которым камера
    i3d MoveFrom {0, 0, 0}; // Origin вокса, с которого камера ушла

    void init_vbo(void);
    void redraw_borders_x(void);
    void redraw_borders_z(void);

  public:
    area(int length, int count, vbo_ext* VBO_pointer);
    ~area(void) {}

    u_int render_indices(void);
    void recalc_borders(void);
    void queue_release(void);
    void append(int);
    void remove(int);
};

} //namespace tr

#endif
