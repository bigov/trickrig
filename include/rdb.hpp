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
#include "rig.hpp"

//#include "objl.hpp"

namespace tr
{

  //## Управление кэшем ригов с поверхности одного уровня LOD
  class rdb
  {
    private:
      std::map<i3d, rig> MapRigs {}; // карта поверхности

      int yMin = -100;  // временное ограничение рабочего пространства
      int yMax = 100;

      // --Level--Of--Details--
      int lod = 1; // размер стороны элементов в LOD = 1

      // Кэш адресов блоков данных в VBO, вышедших за границу рендера
      std::forward_list<GLsizeiptr> CachedOffset {};

      // Карта размещения cнипов по адресам в VBO
      std::unordered_map<GLsizeiptr, snip*> VisibleSnips {};

      vbo VBOdata = {GL_ARRAY_BUFFER};       // VBO вершин поверхности
      glsl Prog3d {};                        // GLSL программа шейдеров

      GLuint space_vao = 0;                  // ID VAO
      u_int render_points = 0;             // число точек передаваемых в рендер

      void _load_16x16_obj(void);
      void side_place(std::vector<snip>&, const f3d&); // разместить данные в VBO буфере
      void side_remove(std::vector<snip>&);            // убрать данные из рендера
      void clear_cashed_snips(void);                   // очистка промежуточного кэша
      void sides_set(rig*);                   // настройка боковых сторон
      void side_make(const std::array<glm::vec4, 4>&, snip&);    // настройка боковой стороны
      void set_Zp(rig*, rig*);
      void set_Zn(rig*, rig*);
      void set_Xp(rig*, rig*);
      void set_Xn(rig*, rig*);
      void make_Zn(std::vector<snip>&, std::vector<snip>&, float, float);
      void make_Zp(std::vector<snip>&, std::vector<snip>&, float, float);
      void make_Xn(std::vector<snip>&, std::vector<snip>&, float, float);
      void make_Xp(std::vector<snip>&, std::vector<snip>&, float, float);
      void append_rig_Yp(const i3d&);

    public:
      rdb(void);                       // конструктор
      void place_in_gpu(rig*);                // разместить данные в VBO буфере
      void remove_from_gpu(rig*);               // убрать риг из рендера
      void draw(void);                 // Рендер кадра

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
