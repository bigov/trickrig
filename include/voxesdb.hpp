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
    voxesdb(vbo_ext* V): std::vector<std::unique_ptr<vox>>{}, pVBO(V) {}

    // Запретить копирование и перенос экземпляров класса
    voxesdb(const voxesdb&) = delete;
    voxesdb& operator=(const voxesdb&) = delete;
    voxesdb(voxesdb&&) = delete;
    voxesdb& operator=(voxesdb&&) = delete;

    vbo_ext* pVBO;             // VBO вершин поверхности

    void append(GLsizeiptr offset);
    void remove(GLsizeiptr offset);
    void load(const i3d& P0);   // загрузить вокс из базы данных в буфер и рендер
    void unload(const i3d& P0); // выгрузить вокс из буфера и из рендера

  private:
    //vecdb_t::iterator FindResult {};

    vox* push_db(std::unique_ptr<vox>);    // Добавить вокс к вектору
    vox* create(const i3d&, int side_len); // создать в указанной точке вокс и записать в БД
    void append_in_vbo(vox*);              // разместить вокс в VBO буфере
    void remove_from_vbo(vox*);            // убрать из VBO
    void recalc_vox_visibility(vox*);
    void recalc_around_visibility(i3d, int side_len);
    vox* get(GLsizeiptr);
    vox* get(const i3d&);
};


}
#endif // VOXESDB_HPP
