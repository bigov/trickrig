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
      std::map<i3d, rig> TplRigs {}; // шаблон карты 16х16
      int yMin = -100;
      int yMax = 100;
      int lod = 1; // размер стороны элементов в текущем LOD

      // Кэш адресов блоков данных в VBO, вышедших за границу рендера
      std::forward_list<GLsizeiptr> CachedOffset {};

      // Карта размещения cнипов по адресам в VBO
      std::unordered_map<GLsizeiptr, snip*> VisibleSnips {};

      vbo VBOdata = {GL_ARRAY_BUFFER};  // VBO вершин поверхности
      glsl Prog3d {};                   // GLSL программа шейдеров

      GLuint space_vao = 0;                 // ID VAO
      GLsizei render_points = 0;            // число точек передаваемых в рендер

      void _load_16x16_obj(void);

    public:
      rdb(void);                              // конструктор
      void put_in_vbo(int, int, int);         // разместить данные в VBO буфере
      void put_in_vbo(rig*, const f3d&);  // разместить данные в VBO буфере
      void remove_from_vbo(int, int, int);    // убрать трик из рендера
      void draw(void);                        // Рендер кадра
      void clear_cashed_snips(void);          // очистка промежуточного кэша

      //bool save(const i3d &, const i3d &);
      void init(int, glm::vec3 = {0,0,0});       // загрузка уровня
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
