//============================================================================
//
// file: scene.hpp
//
// Управление объектами сцены
//
//============================================================================
#ifndef __SCENE_HPP__
#define __SCENE_HPP__

#include "main.hpp"
#include "config.hpp"
#include "space.hpp"
#include "ttf.hpp"

namespace tr
{
  class scene
  {
    private:
      scene(const tr::scene&);
      scene operator=(const tr::scene&);

      GLuint 
        vaoQuad = 0,
        texColorBuffer = 0,
        text = 0,
        frameBuffer = 0;

      tr::TTF ttf {};                   // создание надписей
      tr::space space {};               // виртуальное пространство
      tr::pngImg show_fps {};           // табличка с fps
      tr::glsl screenShaderProgram {};  // шейдерная программа

      void framebuffer_init(void);
      void program2d_init(void);

    public:
      scene(void);
      ~scene(void);
      void draw(const evInput &);
  };

} //namespace
#endif
