/**
 *
 * file: area.hpp
 *
 * Заголовок класса управления элементами области 3D пространства
 *
 */

#ifndef AREA_HPP
#define AREA_HPP

#include "main.hpp"
#include "glsl.hpp"
#include "vbo.hpp"

namespace tr
{

///
/// \brief class area
/// \details Управление картой вокселей
///
class area
{
  private:
    std::map<i3d, voxel> mArea {}; // карта поверхности

    // Карта размещения cнипов по адресам в VBO. Необходима для того,
    // чтобы при перемещениях данных снипов в VBO было проще вносить
    // изменения адреса смещения в буфере.
    std::unordered_map<GLsizeiptr, voxel*> mVBO {};

    int voxel_size;         // Размер вокселя задается при создании объекта класса

    void init_vbo(void);
    voxel* add_voxel(const i3d&);
    void recalc_voxel_visibility(voxel*);
    void recalc_around_visibility(i3d);
    i3d i3d_near(const i3d& P, u_char side);

  public:
    area(int s): voxel_size(s) {}
    ~area(void) {}

    u_int render_indices = 0;  // сумма индексов, необходимых для рендера всех примитивов
    vbo_ext* pVBO = nullptr;   // VBO вершин поверхности

    void voxel_draw(voxel*);   // разместить данные в VBO буфере
    void voxel_wipe(voxel*);   // убрать риг из VBO
    void increase(int);        // добавить объем по индексу снипа
    void decrease(int);        // удалить объем по индексу снипа
    void load_space(vbo_ext*); // загрузка данных в VBO
    voxel* get(const i3d&);
};

} //namespace tr

#endif
