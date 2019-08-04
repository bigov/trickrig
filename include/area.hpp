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
#include "main.hpp"
#include "glsl.hpp"
#include "vbo.hpp"
#include "config.hpp"

namespace tr
{
///
/// \brief class area
/// \details Управление картой воксов
class area
{
  private:
    vbo_ext* pVBO = nullptr;   // VBO вершин поверхности
    std::vector<std::unique_ptr<vox>> VoxBuffer {};
    std::queue<i3d> QueueLoad {}; // адреса загружаемых воксов
    std::queue<i3d> QueueWipe {}; // адреса выгружаемах воксов

    int vox_side_len = 0;   // Длина стороны вокса
    int lod_dist_far = 0;   // Расстояние от камеры до внешней границы LOD, кратное размеру вокса
    i3d Location {0, 0, 0}; // Origin вокса, над которым камера
    i3d MoveFrom {0, 0, 0}; // Origin вокса, с которого камера ушла

    void init_vbo(void);
    vox* add_vox(const i3d&);
    vox* vox_by_i3d(const i3d&);
    vox* vox_by_vbo(GLsizeiptr);
    void recalc_vox_visibility(vox*);
    void recalc_around_visibility(i3d);
    i3d i3d_near(const i3d& P, u_char side);
    void redraw_borders_x(void);
    void redraw_borders_z(void);
    void vox_draw(vox*);            // разместить данные в VBO буфере
    void vox_wipe(vox*);            // убрать из VBO
    void vox_unload(const i3d& P0); // выгрузить вокс из буфера и из рендера
    void vox_load(const i3d& P0);   // загрузить вокс из базы данных в буфер и рендер

  public:
    area(int length, int count);
    ~area(void) {}

    u_int render_indices = 0;  // сумма индексов, необходимых для рендера всех примитивов

    void append(int);                     // добавить объем по индексу снипа
    void remove(int);                     // удалить объем по индексу снипа
    void init(vbo_ext*);                  // загрузка данных в VBO
    void recalc_borders(void);
    void queue_release(void);
};

} //namespace tr

#endif
