//============================================================================
//
// file: scene.cpp
//
// Управление пространством сцены
//
//============================================================================
#include "scene.hpp"

namespace tr
{

///
/// \brief scene::scene
///
scene::scene()
{
  program2d_init();

  // настройка рендер-буфера
  if(!RenderBuffer.init()) ERR("Error on creating Render Buffer.");

  // настройка текстуры для HUD
  glActiveTexture(GL_TEXTURE2);
  glGenTextures(1, &tex_hud_id);
  glBindTexture(GL_TEXTURE_2D, tex_hud_id);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
}


///
/// Инициализация GLSL программы обработки текстуры фреймбуфера.
///
/// \details
/// Текстура фрейм-буфера за счет измения порядка следования координат
/// вершин с 1-2-3-4 на 3-4-1-2 перевернута. При этом верх и низ в сцене
/// меняются местами. Но, благодаря этому, нулевой координатой (0,0) окна
/// становится более привычный верхний-левый угол, а загруженные из файла
/// изображения текстур применяются без дополнительного переворота.
///
void scene::program2d_init(void)
{
  glGenVertexArrays(1, &vao_quad_id);
  glBindVertexArray(vao_quad_id);

  screenShaderProgram.attach_shaders(
        cfg::app_key(SHADER_VERT_SCREEN), cfg::app_key(SHADER_FRAG_SCREEN) );
  screenShaderProgram.use();

  vbo_base VboPosition { GL_ARRAY_BUFFER };
  GLfloat Position[8] = { -1.f, -1.f, 1.f, -1.f, -1.f, 1.f, 1.f, 1.f };
  VboPosition.allocate( sizeof(Position), Position );
  VboPosition.attrib( screenShaderProgram.attrib_location_get("position"),
      2, GL_FLOAT, GL_FALSE, 0, 0);

  vbo_base VboTexcoord { GL_ARRAY_BUFFER };
  GLfloat Texcoord[8] = {
    0.f, 1.f, //3
    1.f, 1.f, //4
    0.f, 0.f, //1
    1.f, 0.f, //2
  };

  VboTexcoord.allocate( sizeof(Texcoord), Texcoord );
  VboTexcoord.attrib( screenShaderProgram.attrib_location_get("texcoord"),
      2, GL_FLOAT, GL_FALSE, 0, 0);

  // GL_TEXTURE1
  glUniform1i(screenShaderProgram.uniform_location_get("texFramebuffer"), 1);
  // GL_TEXTURE2
  glUniform1i(screenShaderProgram.uniform_location_get("texHUD"), 2);

  screenShaderProgram.unuse();
  glBindVertexArray(0);
}


///
/// \brief scene::draw
/// \param ev
///
///  \details  Кадр сцены рендерится в изображение на (2D) "холсте"
/// фреймбуфера, после чего это изображение в виде текстуры накладывается на
/// прямоугольник окна. Курсор и дополнительные (HUD) элементы окна
/// изображаются как наложеные сверху дополнительные текстуры
///
void scene::draw(evInput &ev)
{
  if(AppWin.resized)
    RenderBuffer.resize(static_cast<GLsizei>(AppWin.width), static_cast<GLsizei>(AppWin.height));

  // первый проход рендера во фреймбуфер
  if(AppWin.mode == GUI_HUD3D)
  {
    RenderBuffer.bind();
    WinGui.Space.draw(ev);
    RenderBuffer.unbind();
  }
  else
  {
    WinGui.headband();
  }

  // На текстуре GUI рисуем необходимые элементы
  glBindTexture(GL_TEXTURE_2D, tex_hud_id);
  WinGui.draw(ev);

  // Второй проход - рендер прямоугольника окна с текстурой фреймбуфера
  glBindVertexArray(vao_quad_id);
  glDisable(GL_DEPTH_TEST);
  screenShaderProgram.use();
  // Прицел должен динамически изменять яркость, поэтому используем шейдер
  screenShaderProgram.set_uniform("Cursor", AppWin.Cursor);
  glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
  screenShaderProgram.unuse();
}

} // namespace tr
