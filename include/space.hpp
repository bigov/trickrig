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

namespace tr
{
  class space
  {
    public:
      space(void);
      ~space(void) {}
      void draw(const evInput&);

    private:
      space(const tr::space&);
      space operator=(const tr::space&);

      tr::rdb RigsDb0 {};  // структура 3D пространства LOD-0
      const int g0 = 1;    // масштаб элементов в RigsDb0

      GLuint m_textureObj = 0;

      float rl=0.f, ud=0.f, fb=0.f; // скорость движения по направлениям

      tr::i3d MoveFrom {0, 0, 0},  // координаты рига, на котором "стоим"
              Selected {0, 0, 0};  // координаты рига, на который смотрим

      glm::mat4 MatView {};        // матрица вида
      glm::vec3
        UpWard {0.0, -1.0, 0.0}, // направление наверх
        ViewTo {};               // направление взгляда

      void calc_position(const tr::evInput &);
      void calc_selected_area(glm::vec3 & sight_direction);
      void vbo_allocate_mem(void);
      void recalc_borders(void);
      void redraw_borders_x(void);
      //void redraw_borders_y(void); // TODO
      void redraw_borders_z(void);
  };

} //namespace
#endif
