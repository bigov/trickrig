//============================================================================
//
// file: gen3d.hpp
//
// Заголовок класса управления пространством 3D сцены
//
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

  class Gen3d
  {
    public:
      Gen3d(void){};
      ~Gen3d(void) { delete[] ref_Rig; };
      void init(tr::Rigs*);
      void draw(const evInput&);

    private:
      Gen3d(const tr::Gen3d&);
      Gen3d operator=(const tr::Gen3d&);

      tr::Rigs* rigs = nullptr; // База данных пространства
      tr::Glsl prog3d {};  // GLSL программа шейдеров
      tr::VBO VBO_Inst {}; // VBO буфер атрибутов инстансов
      
      // Буфер обмена индексами из VBO_Inst
      std::list<GLsizeiptr> idx_ref {};

      // Массив ссылок на активные риги, индекс привязан к индексу в VBO_Inst
      tr::Rig** ref_Rig = nullptr;

      // Размер группы атрибутов инстанса в буфере VBO_Inst
      GLsizeiptr InstDataSize = static_cast<GLsizeiptr>(7 * sizeof(GLfloat));

      int   space_i0_length = WIDTH_0;
      int   space_i0_radius = WIDTH_0/2;
      float space_f0_length = static_cast<float>(space_i0_length);
      float space_f0_radius = static_cast<float>(space_i0_radius);

      GLuint vao_3d = 0; // ID VAO
      GLuint m_textureObj = 0;
      GLsizei count = 0; // число отображаемых в сцене инстансов

      float
        rl=0.f, ud=0.f, fb=0.f, // скорость движения по направлениям
        look_a = 3.928f, // азимут (0 - X)
        look_t = -0.276f, // тангаж (0 - горизОнталь, пи/2 - вертикаль)
        k_sense = 4.0f,   // TODO: чувствительность через Config
        k_mouse = 0.002f;

      glm::mat4 MatView {};
      glm::vec3
        ViewFrom {3.f, 4.f, 3.f},
        Selected {},
        MoveFrom {},
        ViewTo {},
        upward {0.0, 1.0, 0.0}; // направление наверх

      void calc_position(const tr::evInput &);
      void space_load(void);
      void vbo_allocate_mem(void);
      void vbo_data_send(float, float, float);
      void inst_write(f3d&, unsigned char);
      void recalc_borders(void);
      void reduce_keys(void);
      void cutback(void);
      void recalc_border_x(float, float, float);
      void recalc_border_z(float, float, float);
  };

} //namespace
#endif
