//=============================================================================
//
// file: rigs.hpp
//
// Элементы формирования пространства
//
//=============================================================================
#ifndef RDB_HPP
#define RDB_HPP

#include "main.hpp"
#include "glsl.hpp"
#include "vbo.hpp"
//#include "config.hpp"

//#include "objl.hpp"

namespace tr
{

///
/// \brief The rdb class
/// \details Управление кэшем ригов с поверхности одного уровня LOD
///
class rdb
{
  private:
    std::map<i3d, rig> MapRigs {}; // карта поверхности

    // Карта размещения cнипов по адресам в VBO. Необходима для того,
    // чтобы при перемещениях данных снипов в VBO было проще вносить
    // изменения адреса смещения в буфере.
    std::unordered_map<GLsizeiptr, box*> Visible {};

    int yMin = -100;  // временное ограничение рабочего пространства
    int yMax = 100;

    // --Level--Of--Details--
    int lod = 1; // размер стороны элементов в LOD = 1

    void _load_16x16_obj(void);
    void init_vbo(void);
    rig* gen_rig(const i3d&);
    void visibility_recalc_rigs(rig* R0);
    void visibility_recalc(i3d P0);

  public:
    rdb(void) {}
    ~rdb(void) {}

    u_int render_points = 0;           // число точек передаваемых в рендер
    vbo_ext* VBO = nullptr;            // VBO вершин поверхности

    void rig_draw(rig*);               // разместить данные в VBO буфере
    void rig_wipe(rig*);               // убрать риг из VBO
    void increase(unsigned int);       // добавить объем по индексу снипа
    void decrease(unsigned int);       // удалить объем по индексу снипа

    //bool save(const i3d &, const i3d &);
    void load_space(vbo_ext* vbo, int l_o_d, const glm::vec3& Position);    // загрузка уровня
    rig* get(const i3d &);

};

} //namespace tr

#endif
