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
#include "vbo.hpp"
#include "config.hpp"

namespace tr
{

class voxesdb: public std::vector<std::unique_ptr<vox>>
{
public:
  voxesdb(void): std::vector<std::unique_ptr<vox>>{}{}

  vox* vox_add_in_db(const i3d&, int side_len); // создать в указанной точке вокс и записать в БД
  vox* push_back(std::unique_ptr<vox>);
  vox* vox_by_vbo(GLsizeiptr);
  vox* vox_by_i3d(const i3d&);
  void recalc_vox_visibility(vox*);
  void recalc_around_visibility(i3d, int side_len);
};

///
/// \brief class area
/// \details Управление картой воксов
class area
{
  public:
    explicit area(int length, int count, const glm::vec3 &ViewFrom, vbo_ext* V);
    ~area(void) {}

    // Запретить копирование и перенос экземпляров класса
    area(const area&) = delete;
    area& operator=(const area&) = delete;
    area(area&&) = delete;
    area& operator=(area&&) = delete;

    u_int get_render_indices(void);
    void recalc_borders(const glm::vec3& ViewFrom);
    void append(u_int);             // добавить объем по индексу поверхности
    void remove(u_int);             // удалить объем по индексу поверхности

  private:
    voxesdb Voxes {};
    vbo_ext* pVBO = nullptr;      // VBO вершин поверхности

    std::queue<i3d> QueueLoad {}; // адреса загружаемых воксов
    std::queue<i3d> QueueWipe {}; // адреса выгружаемах воксов

    u_int render_indices = 0;     // сумма индексов, необходимых для рендера всех примитивов
    int vox_side_len = 0;         // Длина стороны вокса
    int lod_dist_far = 0;         // Расстояние от камеры до внешней границы LOD, кратное размеру вокса
    i3d Location {0, 0, 0};       // Origin вокса, над которым камера
    i3d MoveFrom {0, 0, 0};       // Origin вокса, с которого камера ушла

    void queue_release(void);
    void redraw_borders_x(void);
    void redraw_borders_z(void);
    void vox_load(const i3d& P0);   // загрузить вокс из базы данных в буфер и рендер
    void vox_unload(const i3d& P0); // выгрузить вокс из буфера и из рендера
    void vox_draw(vox*);            // разместить вокс в VBO буфере
    void vox_wipe(vox*);            // убрать из VBO
};



} //namespace tr

#endif
