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
    voxesdb (vbo_ext* V);

    // Запретить копирование и перенос экземпляров класса
    voxesdb (const voxesdb&) = delete;
    voxesdb& operator= (const voxesdb&) = delete;
    voxesdb (voxesdb&&) = delete;
    voxesdb& operator= (voxesdb&&) = delete;

    void append (GLsizeiptr offset);
    void remove (GLsizeiptr offset);
    void load (int x, int z);         // загрузить воксы из базы данных в рендер
    void vbo_truncate(int x, int z);  // Удаление вокса из VBO

  private:
    //vecdb_t::iterator FindResult {};
    vbo_ext* pVBO;              // VBO вершин поверхности

    // В контрольный массив (GpuMap) записываются координаты Origin видимых сторон воксов,
    // переданных в VBO в порядке их размещения. Соответственно, размер массива должен
    // соотвествовать зарезервированному размеру VBO. Адрес размещения определяется
    // умножением значения индекса элемента в массиве на размер блока данных стороны.
    // Так как все видимые стороны вокса одновременно размещается GPU и одновременно
    // удаляются из нее, то нет необходимости различать каждую из сторон вокса - для каждой
    // в массив заносится одино и то-же значение координта Origin.
    std::vector<i3d> GpuMap {};             // Контрольный массив

    void append_in_vbo (vox*);              // разместить вокс в VBO буфере
    void remove_from_vbo (vox*);            // убрать из VBO
    void recalc_vox_visibility (vox*);
    void recalc_around_visibility (i3d, int side_len);
    vox* get (GLsizeiptr);
    vox* get (const i3d&);

    void vbo_expand(uchar* data, uchar n, const i3d& P); // Размещение вокса в VBO
    void vox_create(const i3d& P, int vox_side_len);         // Создание вокса
};


}
#endif // VOXESDB_HPP
