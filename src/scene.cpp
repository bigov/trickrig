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
/// \brief scene::~scene
///
scene::~scene(void)
{
  WinGui = nullptr;
  screenShaderProgram = nullptr;
}


///
/// \brief scene::scene
/// \details
/// Инициализация GLSL программы обработки текстуры фреймбуфера.
///
/// Текстура фрейм-буфера за счет измения порядка следования координат
/// вершин с 1-2-3-4 на 3-4-1-2 перевернута - верх и низ в сцене
/// меняются местами. Благодаря этому, нулевой координатой (0,0) окна
/// становится более привычный верхний-левый угол, и загруженные из файла
/// изображения текстур применяются без дополнительного переворота.
///
scene::scene(void)
{
  GLfloat Position[8] = { // XY координаты вершин
    -1.f,-1.f,
     1.f,-1.f,
    -1.f, 1.f,
     1.f, 1.f
  };

  GLfloat Texcoord[8] = { // UV координаты текстуры
    0.f, 1.f, //3
    1.f, 1.f, //4
    0.f, 0.f, //1
    1.f, 0.f, //2
  };

  screenShaderProgram = std::make_unique<glsl>();
  WinGui = std::make_unique<gui>();

  glGenVertexArrays(1, &vao_quad_id);
  glBindVertexArray(vao_quad_id);

  screenShaderProgram->attach_shaders(
        cfg::app_key(SHADER_VERT_SCREEN), cfg::app_key(SHADER_FRAG_SCREEN) );
  screenShaderProgram->use();

  vbo_base VboPosition { GL_ARRAY_BUFFER };

  VboPosition.allocate( sizeof(Position), Position );
  VboPosition.attrib( screenShaderProgram->attrib_location_get("position"),
      2, GL_FLOAT, GL_FALSE, 0, 0);

  vbo_base VboTexcoord { GL_ARRAY_BUFFER };

  VboTexcoord.allocate( sizeof(Texcoord), Texcoord );
  VboTexcoord.attrib( screenShaderProgram->attrib_location_get("texcoord"),
      2, GL_FLOAT, GL_FALSE, 0, 0);

  // GL_TEXTURE1
  glUniform1i(screenShaderProgram->uniform_location_get("texFramebuffer"), 1);
  // GL_TEXTURE2
  glUniform1i(screenShaderProgram->uniform_location_get("texHUD"), 2);

  screenShaderProgram->unuse();
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
  WinGui->draw(ev);

  // Рендер окна с текстурами фреймбуфера и GIU
  glBindVertexArray(vao_quad_id);
  glDisable(GL_DEPTH_TEST);
  screenShaderProgram->use();
  screenShaderProgram->set_uniform("Cursor", AppWin.Cursor);
  glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
  screenShaderProgram->unuse();
}

} // namespace tr
