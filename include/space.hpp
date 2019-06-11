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
  class space
  {
    public:
      space(void);
      ~space(void);
      void init(void);
      void draw(evInput &);

    private:
      space(const space &);
      space operator=(const space &);

      // GLSL control
      std::unique_ptr<glsl> Prog3d = nullptr;  // GLSL программа шейдеров
      glm::vec3 light_direction {}; // направление освещения
      glm::vec3 light_bright {};    // яркость света
      // Индексы вершин подсвечиваемого вокселя, на который направлен курсор (центр экрана)
      int id_point_0 = 0;           // индекс начальной вершины
      int id_point_8 = 0;           // индекс последней вершины

      // GPU control
      GLuint vao_id = 0;                               // VAO ID
      vbo_ext VBO {GL_ARRAY_BUFFER};                   // VBO вершин поверхности
      vbo_base VBOindex = { GL_ELEMENT_ARRAY_BUFFER }; // индексный буфер
      GLuint texture_id = 0;

      // LOD control
      std::unique_ptr<area> Area4 = nullptr; // Управление пространством вокселей
      const int size_v4 = 64;         // размер стороны вокселя
      float speed_v4 = size_v4*2.f;   // скорость передвижения в пространстве
      const int dist_b4 = 24;          // число элементов от камеры до отображаемой границы

      // Camera control
      float rl=0.f, ud=0.f, fb=0.f; // скорость движения по направлениям
      glm::mat4 MatView {};         // матрица вида
      glm::vec3
        UpWard {0.0, -1.0, 0.0},    // направление наверх
        ViewTo {};                  // направление взгляда

      void load_texture(unsigned gl_texture_index, const std::string& fname);
      void init_vao(void);
      void calc_position(evInput&);
      void render_3d_space(void);    // Рендер 3D сцены
  };

} //namespace
#endif
