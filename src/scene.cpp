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
  framebuffer_init();

  // настройка текстуры HUD
  glActiveTexture(GL_TEXTURE2);
  glGenTextures(1, &tex_hud_id);
  glBindTexture(GL_TEXTURE_2D, tex_hud_id);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
}


///
/// \brief scene::~scene
///
scene::~scene(void)
{
  glBindFramebuffer(GL_FRAMEBUFFER, 0);
  glDeleteFramebuffers(1, &fbuf_id);
}


///
/// Начальная настройка фрейм-буфера
///
void scene::framebuffer_init(void)
{
  glGenFramebuffers(1, &fbuf_id);
  glBindFramebuffer(GL_FRAMEBUFFER, fbuf_id);

  glActiveTexture(GL_TEXTURE1);                // настройка текстуры рендера
  glGenTextures(1, &tex_color_id);
  glBindTexture(GL_TEXTURE_2D, tex_color_id);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

  glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, tex_color_id, 0);

  glGenRenderbuffers(1, &rbuf_id);             // рендер-буфер (глубина и стенсил)
  glBindRenderbuffer(GL_RENDERBUFFER, rbuf_id);
  glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, rbuf_id);

  glBindRenderbuffer(GL_RENDERBUFFER, 0);
  glBindFramebuffer(GL_FRAMEBUFFER, 0);

}


///
/// \brief Перестрока размера фреймбуфера и связанных с ним текстур
///
void scene::framebuffer_resize(void)
{
  GLint l_o_d = 0, frame = 0;

  // пересчет позции координат прицела (центр окна)
  AppWin.Cursor.x = static_cast<float>(AppWin.width/2) + 0.5f;
  AppWin.Cursor.y = static_cast<float>(AppWin.height/2) + 0.5f;

  // пересчет матрицы проекции
  AppWin.aspect = static_cast<float>(AppWin.width) / static_cast<float>(AppWin.height);
  MatProjection = glm::perspective(1.118f, AppWin.aspect, 0.01f, 1000.0f);

  GLsizei
      w = static_cast<GLsizei>(AppWin.width),
      h = static_cast<GLsizei>(AppWin.height);

  // пересчет Viewport
  glViewport(0, 0, w, h);

  // настройка размера текстуры рендера фреймбуфера
  glBindTexture(GL_TEXTURE_2D, tex_color_id);
  glTexImage2D(GL_TEXTURE_2D, l_o_d, GL_RGBA, w, h, frame, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);

  // настройка размера рендербуфера
  glBindRenderbuffer(GL_RENDERBUFFER, rbuf_id);
  glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, w, h);
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

    vbo VboPosition { GL_ARRAY_BUFFER };
    GLfloat Position[8] = { -1.f, -1.f, 1.f, -1.f, -1.f, 1.f, 1.f, 1.f };
    VboPosition.allocate( sizeof(Position), Position );
    VboPosition.attrib( screenShaderProgram.attrib_location_get("position"),
        2, GL_FLOAT, GL_FALSE, 0, 0);

    vbo VboTexcoord { GL_ARRAY_BUFFER };
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
    if(AppWin.resized) framebuffer_resize();

    // первый проход рендера во фреймбуфер
    if(AppWin.mode == GUI_HUD3D)
    {
      glBindFramebuffer(GL_FRAMEBUFFER, fbuf_id);
      WinGui.Space.draw(ev);
      glBindFramebuffer(GL_FRAMEBUFFER, 0);
    }
    else // В режиме настройки 3D текстуру фреймбуфера заполняем заставкой
    {
      glBindTexture(GL_TEXTURE_2D, tex_color_id);
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
