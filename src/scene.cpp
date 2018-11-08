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

    // Загрузка символов для отображения fps
    ttf.init(tr::cfg::get(TTF_FONT), 9);
    ttf.load_chars( L"fps: 0123456789" );
    ttf.set_cursor( 2, 1 );
    ttf.set_color( 0x18, 0x18, 0x18 );
    FpsDisplay.w = 46;
    FpsDisplay.h = 15;
    FpsDisplay.size = static_cast<size_t>( FpsDisplay.w * FpsDisplay.h ) * 4;
    //show_fps.img.assign( show_fps.size, 0x00 );

    // Загрузка обрамления окна (HUD) из файла
    image ImgHud = get_png_img(tr::cfg::get(PNG_HUD));

    glGenTextures(1, &tex_hud);
    glActiveTexture(GL_TEXTURE2);
    glBindTexture(GL_TEXTURE_2D, tex_hud);

    GLint level_of_details = 0, frame = 0;
    glTexImage2D(GL_TEXTURE_2D, level_of_details, GL_RGBA,
      ImgHud.w, ImgHud.h, frame, GL_RGBA, GL_UNSIGNED_BYTE, ImgHud.Data.data());

    //glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
    // Clamping to edges is important to prevent artifacts when scaling
    //glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    //glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    
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
    glTexImage2D(GL_TEXTURE_2D, level_of_details, GL_RGBA, tr::GlWin.width,
          tr::GlWin.height, frame, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    glFramebufferTexture2D(
      GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, Eye.texco_buf, 0
    );
    
    //GLuint Eye.rendr_buf;
    glGenRenderbuffers(1, &Eye.rendr_buf);
    glBindRenderbuffer(GL_RENDERBUFFER, Eye.rendr_buf);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8,
      tr::GlWin.width, tr::GlWin.height);

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
    GLfloat Texcoord[] = { 0.f, 0.f, 1.f, 0.f, 0.f, 1.f, 1.f, 1.f };
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
  // окна. Курсор и дополнительные (HUD) элементы сцены изображаются
  // как наложеные сверху дополнительные изображения
  //

    // Первый проход рендера - во фреймбуфер
    glBindFramebuffer(GL_FRAMEBUFFER, Eye.frame_buf);
    space.draw(ev);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    // Табличка с текстом на экране отображается в виде наложенного на GL_TEXTURE1 изображения
    glActiveTexture(GL_TEXTURE1);
    FpsDisplay.Data.clear();
    FpsDisplay.Data.assign(FpsDisplay.size, 0xCD);
    ttf.set_cursor(2,1);
    ttf.write_wstring(FpsDisplay, {L"fps:" + std::to_wstring(ev.fps)});
    FpsDisplay.flip_vert();
    glTexSubImage2D(GL_TEXTURE_2D, 0, 8, tr::GlWin.height - 22,
      FpsDisplay.w, FpsDisplay.h, GL_RGBA, GL_UNSIGNED_BYTE, FpsDisplay.Data.data());

    // Второй проход рендера - по текстуре из фреймбуфера
    glBindVertexArray(vaoQuad);
    glDisable(GL_DEPTH_TEST);
    screenShaderProgram.use();
    screenShaderProgram.set_uniform("Cursor", tr::GlWin.Cursor);
    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
    screenShaderProgram.unuse();

    return;
  }

} // namespace tr

//Local Variables:
//tab-width:2
//End:
