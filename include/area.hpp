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
#include "voxesdb.hpp"

namespace tr
{

///
/// \brief class area
/// \details Управление картой воксов
class area
{
  public:
    explicit area(int length, int count, std::shared_ptr<voxesdb> V, const glm::vec3 &L);
    ~area(void) {}

    // Запретить копирование и перенос экземпляров класса
    area(const area&) = delete;
    area& operator=(const area&) = delete;
    area(area&&) = delete;
    area& operator=(area&&) = delete;

    void recalc_borders(const glm::vec3& ViewFrom);

  private:
    std::shared_ptr<voxesdb> Voxes;

    std::queue<i3d> QueueLoad {}; // адреса загружаемых воксов
    std::queue<i3d> QueueWipe {}; // адреса выгружаемах воксов

    int side_len     = 0;     // Длина стороны вокса
    float f_side_len = 0.f;   // Длина стороны вокса
    int lod_dist = 0;         // Расстояние от камеры до границы LOD, кратное vox_side_len
    i3d Location {0, 0, 0};   // Origin вокса, над которым камера

    void queue_release(void);
    void redraw_borders_x(int, int);
    void redraw_borders_z(int, int);
};



} //namespace tr

#endif
