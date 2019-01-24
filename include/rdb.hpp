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

enum LAY_NAME { LAY_XP, LAY_XN, LAY_YP, LAY_YN, LAY_ZP, LAY_ZN, LAYS_COUNT };

struct snip_ext: public snip
{
   LAY_NAME lay = LAYS_COUNT;
   i3d Origin {0, 0, 0};
};


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
    std::unordered_map<GLsizeiptr, snip*> VisibleSnips {};

    int yMin = -100;  // временное ограничение рабочего пространства
    int yMax = 100;

    // --Level--Of--Details--
    int lod = 1; // размер стороны элементов в LOD = 1

    void _load_16x16_obj(void);
    void side_make_snip(const std::array<glm::vec4, 4>&, snip&, const glm::vec3&); // настройка боковой стороны
    void set_Zp(rig*);
    void set_Zn(rig*);
    void set_Xp(rig*);
    void set_Xn(rig*);
    void make_Yp(std::vector<snip>&);
    void make_Yn(std::vector<snip>&);
    void make_Zn(std::vector<snip>&, std::vector<snip>&, float, float);
    void make_Zp(std::vector<snip>&, std::vector<snip>&, float, float);
    void make_Xn(std::vector<snip>&, std::vector<snip>&, float, float);
    void make_Xp(std::vector<snip>&, std::vector<snip>&, float, float);
    void append_rig_Yp(const i3d&);
    void remove_rig(const i3d&);
    void init_vbo(void);
    void side_display(std::vector<snip>& Side, const f3d& Point);
    void side_wipeoff(std::vector<snip>&); // убрать сторону рига из VBO
    LAY_NAME lay_direction(const glm::vec4&);
    bool is_top(const std::array<glm::vec4, 4>&, size_t);
    bool is_top(std::vector<snip>&, size_t);
    void snip_analyze(snip_ext& S);

    //void snip_update(GLfloat* s_data, const f3d &Point, GLsizeiptr dist); // код метода в конце файла .cpp

  public:
    rdb(void) {}
    ~rdb(void) {}

    u_int render_points = 0;          // число точек передаваемых в рендер
    vbo_ext* VBO = nullptr;           // VBO вершин поверхности

    void rig_display(rig*);             // разместить данные в VBO буфере
    void rig_wipeoff(rig*);            // убрать риг из VBO

    void increase(unsigned int);      // добавить объем по индексу снипа
    void decrease(unsigned int);        // удалить объем по индексу снипа

    void add_yp(const i3d &);
    void add_yn(const i3d&);
    void add_xn(const i3d&);
    void add_xp(const i3d&);
    void add_zn(const i3d&);
    void add_zp(const i3d&);
    void sub_yn(const i3d&);
    void sub_xn(const i3d&);
    void sub_yp(const i3d &);
    void sub_xp(const i3d&);
    void sub_zn(const i3d&);
    void sub_zp(const i3d&);

    //bool save(const i3d &, const i3d &);
    void load_space(vbo_ext* vbo, int l_o_d, const glm::vec3& Position);    // загрузка уровня
    rig* get(const i3d &);

    i3d search_down(int, int, int);
    i3d search_down(double, double, double);
    i3d search_down(float, float, float);
    i3d search_down(const glm::vec3 &);
};

} //namespace tr

#endif
