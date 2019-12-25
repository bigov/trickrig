#ifndef VOXESDB_HPP
#define VOXESDB_HPP

#include "vox.hpp"
#include "glsl.hpp"
#include "vbo.hpp"
#include "config.hpp"

namespace tr
{

// структура для хранения информации в БД
struct vox_data
{
  int y;                       // положение вокса по оси Y (для учета Y-LOD)
  uchar visible_sides_map;    // бинарная маска видимых сторон (для пересчета видимости)
  uchar data[bytes_per_side]; // данные видимых сторон для размещения в VBO
};


class voxesdb: public std::vector<std::unique_ptr<vox>>
{
  public:
    voxesdb (vbo* V);

    // Запретить копирование и перенос экземпляров класса
    voxesdb (const voxesdb&) = delete;
    voxesdb& operator= (const voxesdb&) = delete;
    voxesdb (voxesdb&&) = delete;
    voxesdb& operator= (voxesdb&&) = delete;

    void remove (GLsizeiptr offset);

  private:
    //vecdb_t::iterator FindResult {};
    vbo* pVBO;              // VBO вершин поверхности

    void recalc_vox_visibility (vox*);
    void recalc_around_visibility (i3d, int side_len);
    vox* get (GLsizeiptr);
    vox* get (const i3d&);
    void vox_create(const i3d& P, int vox_side_len);         // Создание вокса
};


}
#endif // VOXESDB_HPP
