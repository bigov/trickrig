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
