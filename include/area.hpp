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
#include "config.hpp"

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

    int voxel_size =   0;   // Длина стороны вокселя
    int area_width =   0;   // ширина области - расстояние от внешней границы
                            // до внутренней или (для нулевой области) до камеры.

    i3d Location {0, 0, 0}; // Origin вокселя, над которым камера
    i3d MoveFrom {0, 0, 0}; // Origin вокселя с которого камера ушла

    void init_vbo(void);
    voxel* add_voxel(const i3d&);
    void recalc_voxel_visibility(voxel*);
    void recalc_around_visibility(i3d);
    i3d i3d_near(const i3d& P, u_char side);
    void redraw_borders_x(void);
    void redraw_borders_z(void);

  public:
    area(int length, int count);
    ~area(void) {}

    u_int render_indices = 0;  // сумма индексов, необходимых для рендера всех примитивов
    vbo_ext* pVBO = nullptr;   // VBO вершин поверхности

    void voxel_draw(voxel*);   // разместить данные в VBO буфере
    void voxel_wipe(voxel*);   // убрать риг из VBO
    void increase(int);        // добавить объем по индексу снипа
    void decrease(int);        // удалить объем по индексу снипа
    void load_space(void);     // загрузка данных из базы в ОЗУ ЦПУ
    voxel* get(const i3d&);
    void init(vbo_ext*);     // загрузка данных в VBO
    void recalc_borders(void);
};

} //namespace tr

#endif
