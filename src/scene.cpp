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
  struct pixel {
    unsigned char r = 0x00;
    unsigned char g = 0x00;
    unsigned char b = 0x00;
    unsigned char a = 0x00;
  };

  /// формирование полупрозрачной полосы в нижней части сцены
  void hud_fill(pixel* ptr)
  {
    int hud_height = 48;
    if(WinGl.height < hud_height) hud_height = WinGl.height;

    size_t i_max = WinGl.width * hud_height; // сколько пикселей заполнить
    pixel h{0x00, 0x88, 0x00, 0x40};         // RGBA цвет заполнения

    size_t i = 0;
    while(i < i_max) *(ptr + i++) = h;

    return;
  }

  //## Конструктор
  //
  scene::scene()
  {

    program2d_init();
    framebuffer_init();

    // Загрузка символов для отображения fps
    ttf.init(tr::cfg::get(TTF_FONT), 10);
    ttf.load_chars( //L"fps: 0123456789" );
      L"ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz: 0123456789" );
    ttf.set_cursor( 2, 1 );
    ttf.set_color( 0x18, 0x18, 0x18, 0xee );
    Label.w = 120;
    Label.h = 50;
    Label.size = static_cast<size_t>( Label.w * Label.h ) * 4;
    //show_fps.img.assign( show_fps.size, 0x00 );

    // Обрамление окна (HUD)
    std::vector<pixel> Hud {};
    Hud.resize(WinGl.width*WinGl.height, {0x00, 0x00, 0x00, 0x00});
    hud_fill(&Hud[0]);

    glGenTextures(1, &tex_hud);
    glActiveTexture(GL_TEXTURE2);
    glBindTexture(GL_TEXTURE_2D, tex_hud);

    GLint level_of_details = 0, frame = 0;

    // Эта текстура растягивается на все окно. Если указать 4-й/5-й параметры
    // не соответствующие размеру окна (в пикселях), то текстура будет
    // равномерно растянута или сжата до размера окна.
    glTexImage2D(GL_TEXTURE_2D, level_of_details, GL_RGBA,
      WinGl.width, WinGl.height, frame, GL_RGBA, GL_UNSIGNED_BYTE, Hud.data());

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
    glTexImage2D(GL_TEXTURE_2D, level_of_details, GL_RGBA, tr::WinGl.width,
          tr::WinGl.height, frame, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    glFramebufferTexture2D(
      GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, Eye.texco_buf, 0
    );
    
    //GLuint Eye.rendr_buf;
    glGenRenderbuffers(1, &Eye.rendr_buf);
    glBindRenderbuffer(GL_RENDERBUFFER, Eye.rendr_buf);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8,
      tr::WinGl.width, tr::WinGl.height);

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

    //GLfloat Texcoord[] = { 0.f, 0.f, 1.f, 0.f, 0.f, 1.f, 1.f, 1.f };

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
  //
  // Кадр сцены рендерится в изображение на (2D) "холсте" фреймбуфера,
  // после чего это изображение в виде текстуры накладывается на прямоугольник
  // окна. Курсор и дополнительные (HUD) элементы окна изображаются
  // как наложеные сверху дополнительные изображения
  //

    // Первый проход рендера - во фреймбуфер
    glBindFramebuffer(GL_FRAMEBUFFER, Eye.frame_buf);
    space.draw(ev);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    // Табличка с текстом на экране отображается в виде
    // наложенного на GL_TEXTURE2 изображения, которое шейдером складывается
    // с изображением трехмерной сцены, отрендереным во фреймбуфере.
    glActiveTexture(GL_TEXTURE2);

    Label.Data.resize(Label.size);
    size_t i = 0;
    while(i < Label.size)
    {
      Label.Data[i++] = 0xCF;
      Label.Data[i++] = 0xFF;
      Label.Data[i++] = 0xCF;
      Label.Data[i++] = 0x88;
    }

    ttf.set_cursor(2, 2);
    ttf.write_wstring(Label, { L"fps:" + std::to_wstring(ev.fps) });

    ttf.set_cursor(2, 14);
    ttf.write_wstring(Label, { L"w:" + std::to_wstring(tr::WinGl.width) });

    ttf.set_cursor(2, 26);
    ttf.write_wstring(Label, { L"h:" + std::to_wstring(tr::WinGl.height) });

    int xpos = 8; // Положение элемента относительно
    int ypos = 8; // верхнего-левого угла окна
    glTexSubImage2D(GL_TEXTURE_2D, 0, xpos, ypos, Label.w, Label.h,
                    GL_RGBA, GL_UNSIGNED_BYTE, Label.Data.data());

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
