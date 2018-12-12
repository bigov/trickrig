/* Filename: scene.hpp
 *
 * Управление объектами сцены
 *
 */
#ifndef SCENE_HPP
#define SCENE_HPP

#include "gui.hpp"
#include "config.hpp"
#include "space.hpp"

namespace tr
{
  class scene
  {
    private:
      scene(const tr::scene&);
      scene operator=(const tr::scene&);

      GLuint vao_quad_id = 0;
      GLuint tex_hud_id = 0;
      tr::gui WinGui {};               // Интерфейс окна
      tr::space Space {};              // виртуальное пространство
      tr::glsl screenShaderProgram {}; // шейдерная программа

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
