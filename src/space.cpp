/**
 *
 * file: space.cpp
 *
 * Управление виртуальным 3D пространством
 *
 * Коды нажатия клавиш:
 *   Ctrl: 285, S: 31
 *
 */

#include "space.hpp"
#include "config.hpp"

namespace tr
{

///
/// \brief space::space
/// \details Формирование 3D пространства
///
space::space(wglfw* OpenGLContext)
{
  assert(nullptr != OpenGLContext);

  OglContext = OpenGLContext;
  light_direction = glm::normalize(glm::vec3(0.3f, 0.45f, 0.4f)); // направление (x,y,z)
  light_bright = glm::vec3(0.99f, 0.99f, 1.00f);                  // цвет        (r,g,b)

  glClearColor(0.5f, 0.69f, 1.0f, 1.0f);
  glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

  glFrontFace(GL_CCW);
  glCullFace(GL_BACK);

  //glEnable(GL_CULL_FACE);    // отключить отображение обратных поверхностей
  glDisable(GL_CULL_FACE); // не отключать отображение обратных поверхностей

  glEnable(GL_DEPTH_TEST);
  glEnable(GL_LINE_SMOOTH);
  glEnable(GL_BLEND);      // поддержка прозрачности
  glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

  // компиляция GLSL программы
  Prog3d.init();
  Prog3d.attach_shaders( cfg::app_key(SHADER_VERT_SCENE), cfg::app_key(SHADER_FRAG_SCENE) );
  Prog3d.use();

  // Заполнить карту атрибутов для более быстрого доступа
  Prog3d.attrib_location_get("position");
  Prog3d.attrib_location_get("color");
  Prog3d.attrib_location_get("normal");
  Prog3d.attrib_location_get("fragment");

  Prog3d.unuse();

  // настройка рендер-буфера
  GLsizei width, height;
  OglContext->get_frame_buffer_size(&width, &height);
  if(!RenderBuffer.init(width, height)) ERR("Error on creating Render Buffer.");
  OglContext->add_size_observer(RenderBuffer); // пересчет буфера при изменении размеров

  // загрузка основной текстуры
  load_texture(GL_TEXTURE0, cfg::app_key(PNG_TEXTURE0));
}


///
/// \brief space::~space
///
space::~space(void)
{
  Prog3d.destroy();
}


///
/// \brief space::enable
///
void space::enable(void)
{
  // Настройка матрицы проекции
  GLsizei width, height;
  OglContext->get_frame_buffer_size(&width, &height);
  auto aspect = static_cast<float>(width) / static_cast<float>(height);
  MatProjection = glm::perspective(fovy, aspect, zNear, zFar);
  OglContext->add_size_observer(*this); //пересчет матрицы проекции при изменении размера

  xpos = width/2;  // Рассчитать координаты центра экрана
  ypos = height/2;

  OglContext->cursor_hide();  // выключить отображение курсора мыши в окне
  OglContext->set_cursor_pos(xpos, ypos);

  // Продолжительная по времени операция - загрузка в память сцены
  if(nullptr == Area4) Area4 = std::make_unique<area>(size_v4, border_dist_b4);

  OglContext->set_cursor_observer(*this);   // Подключить обработчики: курсора мыши
  OglContext->set_button_observer(*this);   //  -- кнопки мыши
  OglContext->set_keyboard_observer(*this); //  -- клавиатуры
  OglContext->set_focuslost_observer(*this);    // Реакция на потерю окном фокуса ввода
  focus_is_on = true;

  on_front = 0; // клавиша вперед
  on_back  = 0; // клавиша назад
  on_right = 0; // клавиша вправо
  on_left  = 0; // клавиша влево
  on_up    = 0; // клавиша вверх
  on_down  = 0; // клавиша вниз
  fb_way   = 0; // движение вперед
  ud_way   = 0; // движение вверх
  rl_way   = 0; // движение в сторону

  ready = true;
}


///
/// \brief space::load_texture
/// \param index
/// \param fname
///
void space::load_texture(unsigned gl_texture_index, const std::string& FileName)
{
  img ImgTex0 { FileName };
  glGenTextures(1, &texture_id);
  glActiveTexture(gl_texture_index); // можно загрузить не меньше 48
  glBindTexture(GL_TEXTURE_2D, texture_id);

  GLint level_of_details = 0;
  GLint frame = 0;
  glTexImage2D(GL_TEXTURE_2D, level_of_details, GL_RGBA,
               static_cast<GLsizei>(ImgTex0.w_summ),
               static_cast<GLsizei>(ImgTex0.h_summ),
               frame, GL_RGBA, GL_UNSIGNED_BYTE, ImgTex0.uchar());

  // Установка опций отрисовки текстур
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
  glGenerateMipmap(GL_TEXTURE_2D);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST_MIPMAP_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
}


///
/// \brief space::calc_position
/// \param ev
/// \param t: время прорисовки кадра в микросекундах
/// \details Расчет положения и направления движения камеры
///
void space::calc_position(void)
{
  Eye.look_a -= Eye.speed_rotate * dx;
  dx = 0.f;
  if(Eye.look_a > dPi) Eye.look_a -= dPi;
  if(Eye.look_a < 0) Eye.look_a += dPi;

  Eye.look_t -= Eye.speed_rotate * dy;
  dy = 0.f;
  if(Eye.look_t > up_max) Eye.look_t = up_max;
  if(Eye.look_t < down_max) Eye.look_t = down_max;

  //if (!space_is_empty(Eye.ViewFrom)) _k *= 0.1f;       // TODO: скорость/туман в воде

  float dist  = Eye.speed_moving *
          static_cast<float>(cycle_time) * size_v4; // Дистанция перемещения
  rl = dist * rl_way;
  fb = dist * fb_way;   // по трем нормалям от камеры
  ud = dist * ud_way;

  // промежуточные скаляры для ускорения расчета координат точек вида
  float
    _ca = static_cast<float>(cosf(Eye.look_a)),
    _sa = static_cast<float>(sinf(Eye.look_a)),
    _ct = static_cast<float>(cosf(Eye.look_t));

  glm::vec3 LookDir {_ca*_ct, sinf(Eye.look_t), _sa*_ct}; //Направление взгляда
  Eye.ViewFrom += glm::vec3(fb *_ca + rl*sinf(Eye.look_a - Pi), ud,  fb*_sa + rl*_ca);
  ViewTo = Eye.ViewFrom + LookDir;

  // Расчет матрицы вида
  MatView = glm::lookAt(Eye.ViewFrom, ViewTo, UpWard);

  // Матрица преобразования
  MatMVP =  MatProjection * MatView;
}


///
/// \brief gui::calc_render_time
///
void space::calc_render_time(void)
{
  std::chrono::time_point<sys_clock> t_frame = sys_clock::now();

  static int _fps = 0;
  static std::chrono::time_point<sys_clock> cycle_start = t_frame;
  static std::chrono::time_point<sys_clock> fps_start = t_frame;
  static const std::chrono::seconds one_second(1);

  _fps++;
  if (t_frame - fps_start >= one_second)
  {
    fps_start = t_frame;
    FPS = _fps;
    _fps = 0;
  }

  // время (в секундах) прошедшее после предыдущего вызова данный функции
  cycle_time = std::chrono::duration_cast<std::chrono::microseconds>(t_frame - cycle_start).count() / 1000000.0;
  cycle_start = t_frame;
}


///
/// Функция, вызываемая из цикла окна для рендера сцены
///
bool space::render(void)
{
  calc_render_time();
  calc_position();
  Area4->recalc_borders();

  glBindVertexArray(Area4->vao_id());
  RenderBuffer.bind();
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
  glEnable(GL_DEPTH_TEST);

  Prog3d.use();   // включить шейдерную программу
  Prog3d.set_uniform("mvp", MatMVP);
  Prog3d.set_uniform("light_direction", light_direction); // направление
  Prog3d.set_uniform("light_bright", light_bright);       // цвет/яркость
  Prog3d.set_uniform("MinId", GLint(id_point_0));         // начальная вершина активного вокселя
  Prog3d.set_uniform("MaxId", GLint(id_point_8));         // последняя вершина активного вокселя

  glEnableVertexAttribArray(Prog3d.Atrib["position"]);    // положение 3D
  glEnableVertexAttribArray(Prog3d.Atrib["color"]);       // цвет
  glEnableVertexAttribArray(Prog3d.Atrib["normal"]);      // нормаль
  glEnableVertexAttribArray(Prog3d.Atrib["fragment"]);    // текстура

  glDrawElements(GL_TRIANGLES, static_cast<GLsizei>(Area4->render_indices()), GL_UNSIGNED_INT, nullptr);

  glDisableVertexAttribArray(Prog3d.Atrib["position"]);
  glDisableVertexAttribArray(Prog3d.Atrib["color"]);
  glDisableVertexAttribArray(Prog3d.Atrib["normal"]);
  glDisableVertexAttribArray(Prog3d.Atrib["fragment"]);

  Prog3d.unuse(); // отключить шейдерную программу
  RenderBuffer.unbind();
  glBindVertexArray(0);

  return check_keys();
}


///
/// \brief space::resize_event
/// \param width
/// \param height
///
void space::resize_event(int width, int height)
{
  // пересчет матрицы проекции
  auto aspect = static_cast<float>(width) / static_cast<float>(height);
  MatProjection = glm::perspective(fovy, aspect, zNear, zFar);
}


///
/// \brief space::cursor_event
/// \param x
/// \param y
///
void space::cursor_event(double x, double y)
{
  // Накапливаем дистанцию смещения мыши между кадрами рендера.
  // Чем больше значение, тем выше скрость поворота камеры.
  dx += static_cast<float>(x - xpos);
  dy += static_cast<float>(y - ypos);

  // После получения значения счетчика восстановить позицию курсора
  OglContext->set_cursor_pos(xpos, ypos);
}


///
/// \brief space::mouse_event
/// \param _button
/// \param _action
/// \param _mods
///
void space::mouse_event(int _button, int _action, int _mods)
{
  mods   = _mods;
  mouse  = _button;
  action = _action;
}


///
/// \brief space::keyboard_event
/// \param _key
/// \param _scancode
/// \param _action
/// \param _mods
///
void space::keyboard_event(int _key, int _scancode, int _action, int _mods)
{
  mouse    = -1;
  key      = _key;
  scancode = _scancode;
  action   = _action;
  mods     = _mods;

  if (PRESS == _action) {
    switch(_key) {
      case KEY_MOVE_UP:
        on_up = 1;
        break;
      case KEY_MOVE_DOWN:
        on_down = 1;
        break;
      case KEY_MOVE_FRONT:
        on_front = 1;
        break;
      case KEY_MOVE_BACK:
        on_back = 1;
        break;
      case KEY_MOVE_LEFT:
        on_left = 1;
        break;
      case KEY_MOVE_RIGHT:
        on_right = 1;
        break;
      default: break;
    }
  } else if (RELEASE == _action) {
    switch(_key) {
      case KEY_MOVE_UP:
        on_up = 0;
        break;
      case KEY_MOVE_DOWN:
        on_down = 0;
        break;
      case KEY_MOVE_FRONT:
        on_front = 0;
        break;
      case KEY_MOVE_BACK:
        on_back = 0;
        break;
      case KEY_MOVE_LEFT:
        on_left = 0;
        break;
      case KEY_MOVE_RIGHT:
        on_right = 0;
        break;
      default: break;
    }
  }

  fb_way = on_front - on_back;
  ud_way = on_down  - on_up;
  rl_way = on_left  - on_right;
}


///
/// \brief space::focus_event
/// \details Потеря окном фокуса равноценно нажатию [Esc]
///
void space::focus_lost_event()
{
  focus_is_on = false;
}


///
/// \brief space::check_keys
/// \param ev
///
/// Скан-коды клавиш:
/// [S] == 31; [C] == 46
bool space::check_keys()
{
  if(!focus_is_on) return false; // если окно не в фокусе

  if((key == KEY_ESCAPE) && (action == RELEASE))
  {
    key    = -1;
    action = -1;
    return false;
  }

  u_int vertex_id = 0;
  RenderBuffer.read_pixel(GLint(xpos), GLint(ypos), &vertex_id);

  id_point_0 = vertex_id - (vertex_id % vertices_per_side);
  id_point_8 = id_point_0 + vertices_per_side - 1;

  //DEBUG: Нажатие на [C] выводит в консоль номер вершины-индикатора
  if((46 == scancode) && (action == PRESS))
  {
    action = -1;
    scancode = -1;
    std::cout << "ID=" << vertex_id << " ";
  }

  if((mouse == MOUSE_BUTTON_LEFT) && (action == PRESS))
  {
    action = -1;
    Area4->append(vertex_id);
  }

  if((mouse == MOUSE_BUTTON_RIGHT) && (action == PRESS))
  {
    action = -1;
    Area4->remove(vertex_id);
  }
  return true;
}

} // namespace tr
