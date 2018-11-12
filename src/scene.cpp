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
  //## Конструктор
  //
  scene::scene()
  {
    program2d_init();
    framebuffer_init();

    // Настройки отображения HUD
    glGenTextures(1, &tex_hud);
    glActiveTexture(GL_TEXTURE2);
    glBindTexture(GL_TEXTURE_2D, tex_hud);
    // Linear filtering usually looks best for text
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    return;
  }

  //## Деструктор
  //
  scene::~scene(void)
  {
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glDeleteFramebuffers(1, &Eye.frame_buf);
    return;
  }

  //## Инициализация фрейм-буфера
  //
  void scene::framebuffer_init(void)
  {
    glGenFramebuffers(1, &Eye.frame_buf);
    glActiveTexture(GL_TEXTURE1);
    glBindFramebuffer(GL_FRAMEBUFFER, Eye.frame_buf);

    glGenTextures(1, &Eye.texco_buf);
    glBindTexture(GL_TEXTURE_2D, Eye.texco_buf);

    GLint level_of_details = 0, frame = 0;
    glTexImage2D(GL_TEXTURE_2D, level_of_details, GL_RGBA,
                 static_cast<GLsizei>(tr::WinGl.width),
                 static_cast<GLsizei>(tr::WinGl.height),
                 frame, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0,
                           GL_TEXTURE_2D, Eye.texco_buf, 0);
    
    //GLuint Eye.rendr_buf;
    glGenRenderbuffers(1, &Eye.rendr_buf);
    glBindRenderbuffer(GL_RENDERBUFFER, Eye.rendr_buf);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8,
                          static_cast<GLsizei>(tr::WinGl.width),
                          static_cast<GLsizei>(tr::WinGl.height));

    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT,
      GL_RENDERBUFFER, Eye.rendr_buf);

    #ifndef NDEBUG //--контроль создания буфера-------------------------------
    assert(
      glCheckFramebufferStatus( GL_FRAMEBUFFER ) == GL_FRAMEBUFFER_COMPLETE
    );
    #endif
    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    return;
  }

  //## Инициализация GLSL программы обработки текстуры из фреймбуфера.
  void scene::program2d_init(void)
  {
    glGenVertexArrays(1, &vaoQuad);
    glBindVertexArray(vaoQuad);

    screenShaderProgram.attach_shaders(
      tr::cfg::get(SHADER_VERT_SCREEN),
      tr::cfg::get(SHADER_FRAG_SCREEN)
    );
    screenShaderProgram.use();

    tr::vbo vboPosition = {GL_ARRAY_BUFFER};
    GLfloat Position[] = { -1.f, -1.f, 1.f, -1.f, -1.f, 1.f, 1.f, 1.f };
    vboPosition.allocate( sizeof(Position), Position );
    vboPosition.attrib( screenShaderProgram.attrib_location_get("position"),
        2, GL_FLOAT, GL_FALSE, 0, nullptr );

    tr::vbo vboTexcoord = {GL_ARRAY_BUFFER};

    // Переворачиваем текстуру фрейм-буфера изменив порядок следования
    // координат текстуры с 1-2-3-4 на 3-4-1-2, при этом верх и низ в сцене
    // меняются местами. Благодаря этому нулевой координатой (0,0) окна
    // становится более привычный верхний-левый угол. Кроме того, это
    // позволяет накладывать на окно загруженные изображения без переворота.
    GLfloat Texcoord[] = {
      0.f, 1.f, //3
      1.f, 1.f, //4
      0.f, 0.f, //1
      1.f, 0.f, //2
    };

    vboTexcoord.allocate( sizeof(Texcoord), Texcoord );
    vboTexcoord.attrib( screenShaderProgram.attrib_location_get("texcoord"),
        2, GL_FLOAT, GL_FALSE, 0, nullptr );

    // GL_TEXTURE1
    glUniform1i(screenShaderProgram.uniform_location_get("texFramebuffer"), 1);
    // GL_TEXTURE2
    glUniform1i(screenShaderProgram.uniform_location_get("texHUD"), 2);
    screenShaderProgram.unuse();

    glBindVertexArray(0);

    return;
  }

  //## Рендеринг
  void scene::draw(const evInput& ev)
  {
  // Кадр сцены рендерится в изображение на (2D) "холсте" фреймбуфера,
  // после чего это изображение в виде текстуры накладывается на прямоугольник
  // окна. Курсор и дополнительные (HUD) элементы окна изображаются
  // как наложеные сверху дополнительные изображения

    // Первый проход рендера - во фреймбуфер
    glBindFramebuffer(GL_FRAMEBUFFER, Eye.frame_buf);
    Space.draw(ev);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    glActiveTexture(GL_TEXTURE2);
    glBindTexture(GL_TEXTURE_2D, tex_hud);

    // Если окно изменилось, то перестроить изображение GUI
    if(WinGl.renew)
    {
      GuiImage.make();
      glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA,
                   static_cast<GLint>(WinGl.width),
                   static_cast<GLint>(WinGl.height), 0,
                   GL_RGBA, GL_UNSIGNED_BYTE, GuiImage.data);
      WinGl.renew = false;
    }
    else
    {
      GuiImage.update();
    }

    // Второй проход рендера - по текстуре из фреймбуфера
    glBindVertexArray(vaoQuad);
    glDisable(GL_DEPTH_TEST);
    screenShaderProgram.use();
    screenShaderProgram.set_uniform("Cursor", WinGl.Cursor);
    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
    screenShaderProgram.unuse();

    return;
  }

} // namespace tr

//Local Variables:
//tab-width:2
//End:
