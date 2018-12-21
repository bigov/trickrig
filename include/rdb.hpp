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
      int tpl_1_side = 16; // длина стороны шаблона
      std::map<i3d, rig> TplRigs_1 {}; // шаблон карты поверхности

      // Кэш адресов блоков данных в VBO, вышедших за границу рендера
      std::forward_list<GLsizeiptr> CachedOffset {};

      // Карта размещения cнипов по адресам в VBO
      std::unordered_map<GLsizeiptr, snip*> VisibleSnips {};

      vbo VBOdata = {GL_ARRAY_BUFFER};       // VBO вершин поверхности
      glsl Prog3d {};                        // GLSL программа шейдеров

      GLuint space_vao = 0;                  // ID VAO
      GLsizei render_points = 0;             // число точек передаваемых в рендер

      void _load_16x16_obj(void);
      void put_in_vbo(rig*, const f3d&);     // разместить данные в VBO буфере
      void load_space_template(int level);   // загрузка шаблона поверхности

    public:
      rdb(void);                             // конструктор
      void place(int, int, int);             // разместить данные в VBO буфере
      void remove(int, int, int);            // убрать трик из рендера
      void draw(void);                       // Рендер кадра
      void clear_cashed_snips(void);         // очистка промежуточного кэша

      //bool save(const i3d &, const i3d &);
      void load_space(int, const glm::vec3 &);    // загрузка уровня
      void highlight(const i3d &);

      rig* get(const glm::vec3 &);
      rig* get(int x, int y, int z);
      rig* get(const i3d &);

      i3d search_down(int, int, int);
      i3d search_down(double, double, double);
      i3d search_down(float, float, float);
      i3d search_down(const glm::vec3 &);
  };

} //namespace tr

#endif
