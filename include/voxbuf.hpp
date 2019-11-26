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
    vox_buffer(int, int, const i3d, const i3d, const std::list<glsl_attributes>& AtribsList);
    u_int get_render_indices(void);
    void vox_load(const i3d& P0);   // загрузить вокс из базы данных в буфер и рендер
    void vox_unload(const i3d& P0); // выгрузить вокс из буфера и из рендера
    void append(u_int);             // добавить объем по индексу поверхности
    void remove(u_int);             // удалить объем по индексу поверхности
    void init_vao(int border_dist, const std::list<glsl_attributes> &AtribsList);

    GLuint vao_id = 0;              // VAO ID

  private:
    std::vector<std::unique_ptr<vox>> data {};
    std::unique_ptr<vbo_ext> pVBO = nullptr; // VBO вершин поверхности
    u_int render_indices = 0;                // сумма индексов, необходимых для рендера всех примитивов
    int vox_side_len = 0;                    // Длина стороны вокса

    vox* vox_by_vbo(GLsizeiptr);
    vox* vox_by_i3d(const i3d&);
    vox* add_vox_in_db(const i3d&); // создать в указанной точке вокс и записать в БД
    void vox_draw(vox*);            // разместить вокс в VBO буфере
    void vox_wipe(vox*);            // убрать из VBO
    i3d i3d_near(const i3d& P, u_char side);
    void recalc_around_visibility(i3d);
    void recalc_vox_visibility(vox*);
};


} // namespace tr

#endif // VOXBUF_HPP
