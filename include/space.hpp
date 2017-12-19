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

#include "glsl.hpp"
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

      tr::glsl Prog3d {};   // GLSL программа шейдеров
      tr::vbo VBOdata = {GL_ARRAY_BUFFER};   // атрибуты вершин поверхности
      tr::vbo VBOindex = {GL_ELEMENT_ARRAY_BUFFER}; // индексы вершин

      // Карта cнипов, залитых в VBO
      std::unordered_map<GLsizeiptr, tr::snip*> VisibleSnips {};

      // Кэш блоков данных в VBO, вышедших за границу рендера
      std::forward_list<GLsizeiptr> CashedSnips {};

      GLuint space_vao = 0; // ID VAO
      GLuint m_textureObj = 0;
      GLsizei render_points = 0; // число точек передаваемых в рендер

      float
        rl=0.f, ud=0.f, fb=0.f, // скорость движения по направлениям
        look_a = 3.928f,        // азимут (0 - X)
        //look_t = -0.276f,     // тангаж (0 - горизОнталь, пи/2 - вертикаль)
        look_t = -1.7f,         // тангаж (0 - горизОнталь, пи/2 - вертикаль)
        k_sense = 4.0f,         // TODO: чувствительность через Config
        k_mouse = 0.002f;

      glm::mat4 MatView {};
      glm::vec3
        ViewFrom {0.5f, 5.0f, 0.5f},
        Selected {},
        MoveFrom {},
        ViewTo {},
        UpWard {0.0, 1.0, 0.0}; // направление наверх

      void calc_position(const tr::evInput &);
      void calc_selected_area(glm::vec3 & sight_direction);
      void upload_vbo(void);
      void vbo_allocate_mem(void);
      void vbo_data_send(float, float, float);
      void recalc_borders(void);
      void clear_cashed_snips(void);
      void recalc_border_x(float, float, float);
      void redraw_borders_x(void);
      //void redraw_borders_y(void); // TODO
      void redraw_borders_z(void);
  };

} //namespace
#endif
