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
    std::unordered_map<GLsizeiptr, snip*> VisibleSnips {}; // Карта размещения cнипов по адресам в VBO
    u_int render_points = 0;                               // число точек передаваемых в рендер

    int yMin = -100;  // временное ограничение рабочего пространства
    int yMax = 100;

    // --Level--Of--Details--
    int lod = 1; // размер стороны элементов в LOD = 1

    glsl Prog3d {};                   // GLSL программа шейдеров
    GLuint vao_id = 0;                // VAO ID

    void _load_16x16_obj(void);
    void sides_set(rig*);                                        // настройка боковых сторон
    void side_make_snip(const std::array<glm::vec4, 4>&, snip&); // настройка боковой стороны
    void set_Zp(rig*, rig*);
    void set_Zn(rig*, rig*);
    void set_Xp(rig*, rig*);
    void set_Xn(rig*, rig*);
    void make_Zn(std::vector<snip>&, std::vector<snip>&, float, float);
    void make_Zp(std::vector<snip>&, std::vector<snip>&, float, float);
    void make_Xn(std::vector<snip>&, std::vector<snip>&, float, float);
    void make_Xp(std::vector<snip>&, std::vector<snip>&, float, float);
    void append_rig_Yp(const i3d&);
    void remove_rig_Yp(const i3d&);
    void init_vbo(void);
    void snip_place(std::vector<snip>& Side, const f3d& Point);
    void side_remove(std::vector<snip>&); // убрать сторону рига из VBO
    //void snip_update(GLfloat* s_data, const f3d &Point, GLsizeiptr dist); // код метода в конце файла .cpp

  public:
    rdb(void);                            // конструктор
    void render(void);                    // Рендер кадра

    vbo_ext VBO = {GL_ARRAY_BUFFER};      // VBO вершин поверхности

    void rig_place(rig*);                 // разместить данные в VBO буфере
    void rig_remove(rig*);                // убрать риг из VBO

    void add_x(const i3d &);
    void add_y(const i3d &);
    void add_z(const i3d &);
    void sub_x(const i3d &);
    void sub_y(const i3d &);
    void sub_z(const i3d &);

    //bool save(const i3d &, const i3d &);
    void load_space(int, const glm::vec3 &);    // загрузка уровня
    void highlight(const i3d &);

    rig* get(const i3d &);

    i3d search_down(int, int, int);
    i3d search_down(double, double, double);
    i3d search_down(float, float, float);
    i3d search_down(const glm::vec3 &);
};

} //namespace tr

#endif
