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

      GLuint vaoQuad = 0;
      GLuint tex_hud = 0;
      tr::gui GUI {};                  // Интерфейс окна
      tr::ttf TTF10 {};                // создание надписей
      tr::space Space {};              // виртуальное пространство
      tr::image Label {100, 50};       // табличка с fps
      tr::glsl screenShaderProgram {}; // шейдерная программа

      void framebuffer_init(void);
      void program2d_init(void);
      void build_gui(std::vector<UCHAR>&);

    public:
      scene(void);
      ~scene(void);
      void draw(const evInput &);
  };

} //namespace
#endif
