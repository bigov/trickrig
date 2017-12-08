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
//     7(-+-)~~~~~~~~2(++-)
//    /|            /|
//   / |           / |
//  6(-++)~~~~~~~~3(+++)
//  |  |          |  |
//  |  |          |  |
//  |  4(---)~~~~~|~~1(+--)
//  | /           | /
//  |/            |/
//  5(--+)~~~~~~~~0(+-+)
//
//============================================================================
#ifndef __GEN3D_HPP__
#define __GEN3D_HPP__

#include "main.hpp"
#include "config.hpp"
#include "glsl.hpp"
#include "vbo.hpp"
#include "rigs.hpp"

namespace tr
{

  class Space
  {
    public:
      Space(void);
      ~Space(void);
      void draw(const evInput&);

    private:
      Space(const tr::Space&);
      Space operator=(const tr::Space&);

      tr::Rigs RigsDb0 {};  // структура 3D пространства LOD-0
      tr::Glsl Prog3d {};   // GLSL программа шейдеров
      tr::VBO VBOsurf = {GL_ARRAY_BUFFER};   // атрибуты вершин поверхности
      tr::VBO VBOsurfIdx = {GL_ELEMENT_ARRAY_BUFFER}; // индексы вершин

      // Буфер обмена индексами из VBO_Inst
      std::list<GLsizeiptr> cashe_vbo_ptr {};

      // Массив ссылок на риги, отбражаемые на экране. Массив индексируется
      // поррядковыми номерами блоков данных в VBO_Inst
      tr::Rig** visible_rigs = nullptr;

      // 14 чисел float (4 координаты + 4 нормаль + 4 цвет + 2 текстура)
      GLsizeiptr BytesByVertex = static_cast<GLsizeiptr>(14 * sizeof(GLfloat));

      // четырехугольник в VBO
      GLsizeiptr BytesByQuad = 4 * BytesByVertex; // размер (в байтах) группы атрибутов

      int   space_i0_length = WIDTH_0;
      int   space_i0_radius = WIDTH_0/2;
      float space_f0_length = static_cast<float>(space_i0_length);
      float space_f0_radius = static_cast<float>(space_i0_radius);

      GLuint space_vao = 0; // ID VAO
      GLuint m_textureObj = 0;
      GLsizei count_idx = 0; // число индексов для построения поверхности
      GLsizei count_vtc = 0; // количество индексируемых вершин поверхности

      float
        rl=0.f, ud=0.f, fb=0.f, // скорость движения по направлениям
        look_a = 3.928f,        // азимут (0 - X)
        //look_t = -0.276f,       // тангаж (0 - горизОнталь, пи/2 - вертикаль)
        look_t = -1.7f,       // тангаж (0 - горизОнталь, пи/2 - вертикаль)
        k_sense = 4.0f,         // TODO: чувствительность через Config
        k_mouse = 0.002f;

      glm::mat4 MatView {};
      glm::vec3
        ViewFrom {0.f, 14.f, 0.f},
        Selected {},
        MoveFrom {},
        ViewTo {},
        upward {0.0, 1.0, 0.0}; // направление наверх

      void db_connect(void);
      void calc_position(const tr::evInput &);
      void calc_selected_area(glm::vec3 & sight_direction);
      void upload_vbo(void);
      void vbo_allocate_mem(void);
      void vbo_data_send(float, float, float);
      void recalc_borders(void);
      void reduce_keys(void);
      void cutback(void);
      void recalc_border_x(float, float, float);
      void recalc_border_z(float, float, float);
  };

} //namespace
#endif
