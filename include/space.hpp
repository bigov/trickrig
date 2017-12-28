//============================================================================
//
// file: space.hpp
//
// Заголовок класса управления виртуальным пространством
//    pY
//    |
//    |_____ pX
//    /
//   /pZ
//
//     4~~~~~~~~~~~~~5
//    /|            /|
//   / |           / |
//  7~~~~~~~~~~~~~6  |
//  |  |          |  |
//  |  |          |  |
//  |  0~~~~~~~~~~|~~3
//  | /           | /
//  |/            |/
//  1~~~~~~~~~~~~~2
//
//============================================================================
#ifndef __SPACE_HPP__
#define __SPACE_HPP__

#include "rigs.hpp"

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

      tr::rigs RigsDb0 {};   // структура 3D пространства LOD-0
      const float g0 = 1.0f; // масштаб элементов в RigsDb0

      GLuint m_textureObj = 0;

      float rl=0.f, ud=0.f, fb=0.f; // скорость движения по направлениям


      glm::mat4 MatView = {};
      glm::vec3
        Selected = {},
        MoveFrom = {},
        ViewTo = {},
        UpWard = {0.0, 1.0, 0.0}; // направление наверх

      void calc_position(const tr::evInput &);
      void calc_selected_area(glm::vec3 & sight_direction);
      void upload_vbo(void);
      void vbo_allocate_mem(void);
      void vbo_data_send(float, float, float);
      void recalc_borders(void);
      void recalc_border_x(float, float, float);
      void redraw_borders_x(void);
      //void redraw_borders_y(void); // TODO
      void redraw_borders_z(void);
  };

} //namespace
#endif
