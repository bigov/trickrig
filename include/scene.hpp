/* Filename: scene.hpp
 *
 * Управление объектами сцены
 *
 */

#ifndef SCENE_HPP
#define SCENE_HPP

#include "gui.hpp"
#include "config.hpp"

namespace tr
{

  class scene
  {
    private:
      scene(const tr::scene&);
      scene operator=(const tr::scene&);

      GLuint fbuf_id      = 0;         // id фрейм-буфера рендера сцены
      GLuint rbuf_id      = 0;         // id рендер-буфера
      GLuint text_fbuf_id = 0;         // id основной тектуры фрейм-буфера
      GLuint tex_hud_id   = 0;         // id тектуры HUD
      GLuint vao_quad_id  = 0;
      gui WinGui {};               // Интерфейс окна
      glsl screenShaderProgram {}; // шейдерная программа обработки текстуры рендера

      void framebuffer_init(void);
      void framebuffer_resize(void);
      void program2d_init(void);

    public:
      scene(void);
      ~scene(void);
      void draw(evInput&);
  };

} //namespace
#endif
