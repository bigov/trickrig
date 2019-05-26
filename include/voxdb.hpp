/**
 *
 * file: voxdb.hpp
 *
 * Управление ллементами формирования пространства
 */

#ifndef VOXDB_HPP
#define VOXDB_HPP

#include "main.hpp"
#include "glsl.hpp"
#include "vbo.hpp"

namespace tr
{

///
/// \brief The rdb class
/// \details Управление картой вокселей
///
class voxdb
{
  private:
    std::map<i3d, voxel> VoxMap {}; // карта поверхности

    // Карта размещения cнипов по адресам в VBO. Необходима для того,
    // чтобы при перемещениях данных снипов в VBO было проще вносить
    // изменения адреса смещения в буфере.
    std::unordered_map<GLsizeiptr, voxel*> Visible {};

    int yMin = -100;        // временное ограничение рабочего пространства
    int yMax = 100;
    bool caps_lock = false; // ключ поступеньчатого изменения размера боксов
    int lod = 1;            // --Level--Of--Details-- размер стороны элементов в LOD = 1

    void _load_16x16_obj(void);
    void init_vbo(void);
    voxel* add_voxel_on_map(const i3d&);
    void visibility_voxel_recalc(voxel*);
    void visibility_recalc(i3d);


  public:
    voxdb(void) {}
    ~voxdb(void) {}

    u_int render_indices = 0;      // сумма индексов, необходимых для рендера всех примитивов
    vbo_ext* VBO = nullptr;        // VBO вершин поверхности

    void caps_lock_toggle(void);   // переключить положение caps_lock
    void voxel_draw(voxel*);           // разместить данные в VBO буфере
    void voxel_wipe(voxel*);           // убрать риг из VBO
    void increase(int);   // добавить объем по индексу снипа
    void decrease(int);   // удалить объем по индексу снипа

    //bool save(const i3d &, const i3d &);
    void load_space(vbo_ext* vbo, int l_o_d, const glm::vec3& Position);    // загрузка уровня
    voxel* get(const i3d &);

};

} //namespace tr

#endif
