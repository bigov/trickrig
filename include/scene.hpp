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
  class Scene
  {
    private:
      Scene(const tr::Scene&);
      Scene operator=(const tr::Scene&);

      GLuint 
        vaoQuad = 0,
        texColorBuffer = 0,
        text = 0,
        frameBuffer = 0;

      tr::Config* cfg = nullptr;        // управление настройками
      tr::TTF ttf {};                   // создание надписей
      tr::Space space {};               // виртуальное пространство
      tr::pngImg show_fps {};           // табличка с fps
      tr::Glsl screenShaderProgram {};  // шейдерная программа

      void framebuffer_init(void);
      void program2d_init(void);

    public:
      Scene(tr::Config*);
      ~Scene(void);
      void draw(const evInput &);
  };

} //namespace
#endif
