#ifndef VOXBUF_HPP
#define VOXBUF_HPP

#include "config.hpp"
#include "vbo.hpp"

namespace tr {


///
/// \brief The vox_buffer class
/// \details Буфер воксов предназначен для удобства управления их рендером
///
class vox_buffer
{
  public:
    vox_buffer(int, vbo_ext*, const i3d, const i3d);
    void push_vox(std::unique_ptr<vox>);

    void add_vox(const i3d&);       // создать в указанной точке вокс
    void vox_load(const i3d& P0);   // загрузить вокс из базы данных в буфер и рендер
    void vox_unload(const i3d& P0); // выгрузить вокс из буфера и из рендера
    u_int get_render_indices(void);
    void append(int);               // добавить объем по индексу снипа
    void remove(int);               // удалить объем по индексу снипа
    vox* vox_by_i3d(const i3d&);
    vox* vox_by_vbo(GLsizeiptr);
    void recalc_vox_visibility(vox*);
    void recalc_around_visibility(i3d);

  private:
    std::vector<std::unique_ptr<vox>> data {};
    vbo_ext* pVBO = nullptr;   // VBO вершин поверхности

    u_int render_indices = 0;       // сумма индексов, необходимых для рендера всех примитивов
    int vox_side_len = 0;           // Длина стороны вокса

    void vox_draw(vox*);            // разместить вокс в VBO буфере
    void vox_wipe(vox*);            // убрать из VBO
    i3d i3d_near(const i3d& P, u_char side);
};


} // namespace tr

#endif // VOXBUF_HPP
