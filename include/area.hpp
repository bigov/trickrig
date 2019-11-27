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
    explicit area(int length, int count, std::shared_ptr<voxesdb> V);
    ~area(void) {}

    // Запретить копирование и перенос экземпляров класса
    area(const area&) = delete;
    area& operator=(const area&) = delete;
    area(area&&) = delete;
    area& operator=(area&&) = delete;

    void load(const glm::vec3& ViewFrom);
    void recalc_borders(const glm::vec3& ViewFrom);
    void append(u_int);             // добавить объем по индексу поверхности
    void remove(u_int);             // удалить объем по индексу поверхности

  private:
    std::shared_ptr<voxesdb> Voxes;

    std::queue<i3d> QueueLoad {}; // адреса загружаемых воксов
    std::queue<i3d> QueueWipe {}; // адреса выгружаемах воксов

    int vox_side_len = 0;         // Длина стороны вокса
    int lod_dist_far = 0;         // Расстояние от камеры до внешней границы LOD, кратное размеру вокса
    i3d Location {0, 0, 0};       // Origin вокса, над которым камера
    i3d MoveFrom {0, 0, 0};       // Origin вокса, с которого камера ушла

    void queue_release(void);
    void redraw_borders_x(void);
    void redraw_borders_z(void);
};



} //namespace tr

#endif
