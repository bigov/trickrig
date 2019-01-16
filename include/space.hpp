//============================================================================
//
// file: space.hpp
//
// Заголовок класса управления виртуальным пространством
//
//============================================================================
#ifndef SPACE_HPP
#define SPACE_HPP

#include "rdb.hpp"
#include "framebuf.hpp"

namespace tr
{
  class space
  {
    public:
      space(void);
      ~space(void) {}
      void init3d(void);
      void draw(evInput &);

    private:
      space(const space &);
      space operator=(const space &);

      fb_base BufferRender {};
      //fb_tex BufferPick{};

      glsl Prog3d {};                // GLSL программа шейдеров
      glsl ProgPick {};               // GLSL программа шейдеров для отбора элементов

      const int g1 = 1;              // масштаб элементов в RigsDb0
      GLuint vao_id = 0;             // VAO ID

      vbo_ext VBO {GL_ARRAY_BUFFER}; // VBO вершин поверхности
      rdb RigsDb0 {};                // структура 3D пространства LOD-0
      GLuint texture_id = 0;
      float rl=0.f, ud=0.f, fb=0.f; // скорость движения по направлениям

      i3d MoveFrom {0, 0, 0},       // координаты рига, на котором "стоим"
          Selected {0, 0, 0};       // координаты рига, на который смотрим

      glm::mat4 MatView {};         // матрица вида
      glm::vec3
        UpWard {0.0, -1.0, 0.0},    // направление наверх
        ViewTo {};                  // направление взгляда

      void load_texture(unsigned index, const std::string& fname);
      void glsl_progs_init(void);
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
