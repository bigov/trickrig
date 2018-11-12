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
  /// \brief Создание элементов интерфейса окна
  ///
  /// \details Окно приложения может иметь два состояния - открытое, в котором
  /// происходит основной процесс, и режим настройка/управление (закрытое). В
  /// обоих режимах поверх основного изображения OpenGL сцены накладываются
  /// изображения дополнительных элементов. В первом случае это HUD и
  /// контрольные элементы взаимодействия с контентом сцены, во втором -
  /// элементы управления и настройки приложения.
  ///
  /// Из всех необходимых элементов собирается общее графическое изображение в
  /// виде пиксельного массива и передается для рендера OpenGL изображения в
  /// качестве накладываемой текстуры фрейм-буфера.
  ///
  void scene::build_gui(std::vector<UCHAR>& P)
  {
    if(WinGl.is_open)
    {
      GUI.make_panel(P);
    }
    else
    {
      GUI.obscure(P);
      GUI.make_button(P, L"Open");
    }
    return;
  }

  //## Конструктор
  //
  scene::scene()
  {
    program2d_init();
    framebuffer_init();

    // Загрузка символов для отображения fps
    TTF10.init(tr::cfg::get(TTF_FONT), 10);
    TTF10.set_color( 0x20, 0x20, 0x20, 0xFF );
    TTF10.load_chars(
      L"ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz: 0123456789" );

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

    // Переворачиваем текстуру фрейм-буфера - меняем порядок следования
    // координат текстуры с 1-2-3-4 на 3-4-1-2, при этом верх и низ меняются
    // местами. Благодаря этому точка окна с нулевыми координатами (0,0)
    // перемещается в более привычный верхний-левый угол. Кроме того, это
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

    // Если окно изменилось - перерисовать GUI элементы
    if(WinGl.renew)
    {
      std::vector<UCHAR> TexImg(WinGl.width * WinGl.height * 4, 0x00);
      build_gui(TexImg);
      glBindTexture(GL_TEXTURE_2D, tex_hud);
      glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA,
                   static_cast<GLint>(WinGl.width),
                   static_cast<GLint>(WinGl.height), 0,
                   GL_RGBA, GL_UNSIGNED_BYTE, TexImg.data());
      WinGl.renew = false;
    }

    // Табличка с текстом на экране отображается в виде
    // наложенного на GL_TEXTURE2 изображения, которое шейдером складывается
    // с изображением трехмерной сцены, отрендереным во фреймбуфере.
    if(WinGl.is_open)
    {
      size_t i = 0;
      size_t max = Label.Data.size();
      while(i < max)
      {
        Label.Data[i++] = 0xCF;
        Label.Data[i++] = 0xFF;
        Label.Data[i++] = 0xCF;
        Label.Data[i++] = 0x88;
      }

      TTF10.set_cursor(4, 2);
      TTF10.write_wstring(Label, { L"fps:" + std::to_wstring(ev.fps) });

      TTF10.set_cursor(6, 14);
      TTF10.write_wstring(Label, { L"w:" + std::to_wstring(tr::WinGl.width) });

      TTF10.set_cursor(5, 26);
      TTF10.write_wstring(Label, { L"h:" + std::to_wstring(tr::WinGl.height) });

      int xpos = 8; // Положение элемента относительно
      int ypos = 8; // верхнего-левого угла окна
      glTexSubImage2D(GL_TEXTURE_2D, 0, xpos, ypos,
                      static_cast<GLsizei>(Label.w),
                      static_cast<GLsizei>(Label.h),
                    GL_RGBA, GL_UNSIGNED_BYTE, Label.Data.data());
    }

    // Второй проход рендера - по текстуре из фреймбуфера
    glBindVertexArray(vaoQuad);
    glDisable(GL_DEPTH_TEST);
    screenShaderProgram.use();
    screenShaderProgram.set_uniform("Cursor", tr::WinGl.Cursor);
    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
    screenShaderProgram.unuse();

    return;
  }

} // namespace tr

//Local Variables:
//tab-width:2
//End:
