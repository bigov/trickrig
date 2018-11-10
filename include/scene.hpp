/* Filename: scene.hpp
 *
 * Управление объектами сцены
 *
 */
#ifndef SCENE_HPP
#define SCENE_HPP

#include "main.hpp"
#include "config.hpp"
#include "space.hpp"
#include "ttf.hpp"

namespace tr
{
  struct pixel
  {
    unsigned char r = 0x00;
    unsigned char g = 0x00;
    unsigned char b = 0x00;
    unsigned char a = 0x00;
  };

  class scene
  {
    private:
      scene(const tr::scene&);
      scene operator=(const tr::scene&);

      GLuint vaoQuad = 0;
      GLuint tex_hud = 0;
      tr::ttf TTF10 {};              // создание надписей
      tr::ttf TTF12 {};              // создание подписей
      tr::space Space {};              // виртуальное пространство
      tr::image Label {100, 50};       // табличка с fps
      tr::glsl screenShaderProgram {}; // шейдерная программа

      void framebuffer_init(void);
      void program2d_init(void);
      void hud_fill(std::vector<pixel>&);
      void make_button(std::vector<pixel>&, const std::wstring&);

    public:
      scene(void);
      ~scene(void);
      void draw(const evInput &);
  };

} //namespace
#endif
