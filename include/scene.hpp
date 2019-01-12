/* Filename: scene.hpp
 *
 * Управление объектами сцены
 *
 */

#ifndef SCENE_HPP
#define SCENE_HPP

#include "gui.hpp"
#include "config.hpp"
#include "framebuf.hpp"

namespace tr
{
class scene
{
  private:
    scene(const tr::scene&);
    scene operator=(const tr::scene&);

    //GLuint tex_space_id  = 0;    // id тектуры для рендера фрейм-буфера
    fb_ren RenderBuffer {};

    GLuint tex_hud_id   = 0;     // id тектуры HUD
    GLuint vao_quad_id  = 0;
    gui WinGui {};               // Интерфейс окна
    glsl screenShaderProgram {}; // шейдерная программа обработки текстуры рендера

    void program2d_init(void);

  public:
    scene(void);
    ~scene(void){}
    void draw(evInput&);
};

} //namespace
#endif
