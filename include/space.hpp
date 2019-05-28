/**
 *
 * file: space.hpp
 *
 * Заголовок класса управления виртуальным пространством
 *
 */

#ifndef SPACE_HPP
#define SPACE_HPP

#include "area.hpp"
#include "framebuf.hpp"

namespace tr
{
  static const int lod0 = 9; // число отображаемых элементов до границы Area0

  class space
  {
    public:
      space(void);
      ~space(void);
      void init3d(void);
      void draw(evInput &);

    private:
      space(const space &);
      space operator=(const space &);

      std::unique_ptr<glsl> Prog3d = nullptr;  // GLSL программа шейдеров

      glm::vec3 light_direction {};  // направление освещения
      glm::vec3 light_bright {};     // яркость света

      const int g1 = 1;              // масштаб элементов в RigsDb0
      GLuint vao_id = 0;             // VAO ID

      // Индексы вершин подсвечиваемого вокселя, на который направлен курсор (центр экрана)
      int id_point_0 = 0;            // индекс начальной вершины
      int id_point_8 = 0;            // индекс последней вершины

      vbo_ext VBO {GL_ARRAY_BUFFER};                   // VBO вершин поверхности
      vbo_base VBOindex = { GL_ELEMENT_ARRAY_BUFFER }; // индексный буфер
      GLuint texture_id = 0;

      area Area0 {2};               // Управление пространством вокселей

      float rl=0.f, ud=0.f, fb=0.f; // скорость движения по направлениям

      i3d MoveFrom {0, 0, 0},       // координаты рига, на котором "стоим"
          Selected {0, 0, 0};       // координаты рига, на который смотрим

      glm::mat4 MatView {};         // матрица вида
      glm::vec3
        UpWard {0.0, -1.0, 0.0},    // направление наверх
        ViewTo {};                  // направление взгляда

      void load_texture(unsigned gl_texture_index, const std::string& fname);
      void init_vao(void);
      void calc_position(evInput&);
      void recalc_borders(void);
      void redraw_borders_x(void);
      //void redraw_borders_y(void); // TODO
      void redraw_borders_z(void);
      void render_3d_space(void);    // Рендер 3D сцены
  };

} //namespace
#endif
