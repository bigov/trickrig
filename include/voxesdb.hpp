#ifndef VOXESDB_HPP
#define VOXESDB_HPP

#include "vox.hpp"
#include "glsl.hpp"
#include "vbo.hpp"
#include "config.hpp"

namespace tr
{

//using vecdb_t = std::vector<std::unique_ptr<vox>>;

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
    void load (const i3d& P0);   // загрузить вокс из базы данных в буфер и рендер
    void unload (const i3d& P0); // выгрузить вокс из буфера и из рендера

  private:
    //vecdb_t::iterator FindResult {};
    vbo_ext* pVBO;              // VBO вершин поверхности

    // В этот массив записываются значения Origin видимых сторон воксов в порядке
    // их размещения в VBO. Соответственно, размер массива должен соотвествовать
    // зарезервированному размеру VBO. Адрес размещения определяется умножением
    // индекса элемента в массиве на размер блока данных одной стороны. Так как
    // все видимые стороны вокса одновременно размещается GPU и одновременно
    // удаляются из нее, то нет необходимости различать каждую из сторон персонально -
    // для каждой из сторон в массив заносится одино и то-же значение Origin вокса.
    std::vector<i3d> GpuMap {};

    vox* push_db (std::unique_ptr<vox>);    // Добавить вокс к вектору
    vox* create (const i3d&, int side_len); // создать в указанной точке вокс и записать в БД
    void append_in_vbo (vox*);              // разместить вокс в VBO буфере
    void remove_from_vbo (vox*);            // убрать из VBO
    void recalc_vox_visibility (vox*);
    void recalc_around_visibility (i3d, int side_len);
    vox* get (GLsizeiptr);
    vox* get (const i3d&);
};


}
#endif // VOXESDB_HPP
