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

namespace tr
{
std::unique_ptr<frame_buffer> RenderBuffer = nullptr; // рендер-буфер окна


///
/// \brief space::space
/// \details Формирование 3D пространства
///
space_3d::space_3d(std::shared_ptr<trgl>& pGl): OGLContext(pGl)
{
  render_indices.store(0);
  ViewFrom = std::make_shared<glm::vec3> ();

  // настройка рендер-буфера
  GLsizei width, height;
  OGLContext->get_frame_size(&width, &height);
  RenderBuffer = std::make_unique<frame_buffer>(width, height);

  light_direction = glm::normalize(glm::vec3(0.3f, 0.45f, 0.4f)); // направление (x,y,z)
  light_bright = glm::vec3(0.99f, 0.99f, 1.00f);                  // цвет        (r,g,b)

  glClearColor(0.5f, 0.69f, 1.0f, 1.0f);
  glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

  glFrontFace(GL_CCW);
  glCullFace(GL_BACK);

  //glEnable(GL_CULL_FACE); // отключить отображение обратных поверхностей
  glDisable(GL_CULL_FACE);  // обратные поверхности отображать

  glEnable(GL_DEPTH_TEST);
  glEnable(GL_LINE_SMOOTH);
  glEnable(GL_BLEND);      // поддержка прозрачности
  glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

  init_prog_3d();
  load_surf_textures(); // загрузка текстурной карты поверхностей
  OGLContext->add_size_observer(*this); //пересчет при изменении размера

  init_buffers();
}


///
/// \brief space::init_prog_3d
///
void space_3d::init_prog_3d(void)
{
  std::list<std::pair<GLenum, std::string>> Shaders {};
  Shaders.push_back({ GL_VERTEX_SHADER, cfg::app_key(SHADER_VERT_SCENE) });
  Shaders.push_back({ GL_FRAGMENT_SHADER, cfg::app_key(SHADER_FRAG_SCENE) });

  Program3d = std::make_unique<glsl>(Shaders);
  Program3d->use();

  // Заполнить список атрибутов GLSL программы
  Program3d->AtribsList.push_back(
    { Program3d->attrib("position"), 3, GL_FLOAT,GL_FALSE, bytes_per_vertex, 0 * sizeof(GLfloat) });
  Program3d->AtribsList.push_back(
    { Program3d->attrib("color"),    4, GL_FLOAT, GL_TRUE, bytes_per_vertex, 3 * sizeof(GLfloat) });
  Program3d->AtribsList.push_back(
    { Program3d->attrib("normal"),   3, GL_FLOAT, GL_TRUE, bytes_per_vertex, 7 * sizeof(GLfloat) });
  Program3d->AtribsList.push_back(
    { Program3d->attrib("fragment"), 2, GL_FLOAT, GL_TRUE, bytes_per_vertex, 10 * sizeof(GLfloat)});

  glUniform1i(Program3d->uniform("texture_0"), 0);  // glActiveTexture(GL_TEXTURE0)

  Program3d->unuse();
}


///
/// \brief space::init_buffers
/// \details Инициализация VAO и VBO
///
void space_3d::init_buffers(void)
{
  glGenVertexArrays(1, &vao_3d);
  glBindVertexArray(vao_3d);

  // Число сторон куба в объеме с длиной стороны LOD (2*dist_xx) элементов:
  uint n = static_cast<uint>(pow((border_dist_b4 + border_dist_b4 + 1), 3));
  VBO3d.allocate(n * bytes_per_face);          // Размер данных VBO для размещения сторон вокселей:
  VBO3d.set_attributes(Program3d->AtribsList); // настройка положения атрибутов GLSL программы

  // Так как все четырехугольники сторон индексируются одинаково, то индексный массив
  // заполняем один раз "под завязку" и забываем про него. Число используемых индексов
  // будет всегда соответствовать числу элементов, передаваемых в процедру "glDraw..."
  size_t idx_size = static_cast<size_t>(6 * n * sizeof(GLuint)); // Размер индексного массива
  auto idx_data = std::unique_ptr<GLuint[]> {new GLuint[idx_size]};

  GLuint idx[6] = {0, 1, 2, 2, 3, 0};                                // шаблон четырехугольника
  GLuint stride = 0;                                                 // число описаных вершин
  for(size_t i = 0; i < idx_size; i += 6) {                          // заполнить массив для VBO
    for(size_t x = 0; x < 6; x++) idx_data[x + i] = idx[x] + stride;
    stride += 4;                                                     // по 4 вершины на сторону
  }

  vbo VBOindex { GL_ELEMENT_ARRAY_BUFFER };                          // Индексный буфер
  VBOindex.allocate(static_cast<GLsizei>(idx_size), idx_data.get()); // заполнить данными.
  glBindVertexArray(0);
}


///
/// \brief space::enable
///
void space_3d::load(void)
{
  render_indices.store(0);
  // Поток обмена данными с базой. Загрузка карты занимает некоторое время
  data_loader = std::make_unique<std::thread>(db_control, OGLContext, ViewFrom, VBO3d.get_id(), VBO3d.get_size());

  // Настройка матрицы проекции
  GLsizei width, height;
  OGLContext->get_frame_size(&width, &height);
  auto aspect = static_cast<float>(width) / static_cast<float>(height);
  MatProjection = glm::perspective(fovy, aspect, zNear, zFar);

  xpos = width/2;                          // координаты центра экрана
  ypos = height/2;

  OGLContext->cursor_hide();               // выключить отображение курсора мыши в окне
  OGLContext->set_cursor_pos(xpos, ypos);
  OGLContext->set_cursor_observer(*this);  // Подключить обработчики: курсора мыши
  OGLContext->set_mbutton_observer(*this);  //  -- кнопки мыши

  on_front = 0; // клавиша вперед
  on_back  = 0; // клавиша назад
  on_right = 0; // клавиша вправо
  on_left  = 0; // клавиша влево
  on_up    = 0; // клавиша вверх
  on_down  = 0; // клавиша вниз
  fb_way   = 0; // движение вперед
  ud_way   = 0; // движение вверх
  rl_way   = 0; // движение в сторону
}


///
/// \brief space::load_texture
/// \param index
/// \param fname
///
void space_3d::load_surf_textures(void)
{
  // Загрузка текстур поверхностей воксов
  glActiveTexture(GL_TEXTURE0);
  GLuint texture_3d = 0;
  glGenTextures(1, &texture_3d);
  glBindTexture(GL_TEXTURE_2D, texture_3d);
  image ImgTex0 { cfg::app_key(PNG_TEXTURE0) };
  GLint level_of_details = 0;
  GLint frame = 0;
  glTexImage2D(GL_TEXTURE_2D, level_of_details, GL_RGBA,
               static_cast<GLsizei>(ImgTex0.get_width()),
               static_cast<GLsizei>(ImgTex0.get_height()),
               frame, GL_RGBA, GL_UNSIGNED_BYTE, ImgTex0.uchar_t());

  // Установка опций отрисовки
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
  glGenerateMipmap(GL_TEXTURE_2D);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST_MIPMAP_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
}


///
/// \brief space::calc_position
/// \details Расчет положения и направления движения камеры
///
void space_3d::calc_position(void)
{
  look_dir[0] -= speed_rotate * cursor_dx;
  cursor_dx = 0.f;
  if(look_dir[0] > dPi) look_dir[0] -= dPi;
  if(look_dir[0] < 0) look_dir[0] += dPi;

  look_dir[1] -= speed_rotate * cursor_dy;
  cursor_dy = 0.f;
  if(look_dir[1] > up_max) look_dir[1] = up_max;
  if(look_dir[1] < down_max) look_dir[1] = down_max;

  //if (!space_is_empty(Eye.ViewFrom)) _k *= 0.1f;       // TODO: скорость/туман в воде

  // время (в секундах) прошедшее между фреймами
  std::chrono::time_point<sys_clock> t_now = sys_clock::now();
  static auto cycle_start = t_now;
  double frame_time = std::chrono::duration_cast<std::chrono::microseconds>(t_now - cycle_start).count() / 1000000.0;
  cycle_start = t_now;

  float dist  = speed_moving * static_cast<float>(frame_time); // Дистанция перемещения
  rl = dist * rl_way;
  fb = dist * fb_way;   // по трем нормалям от камеры
  ud = dist * ud_way;

  // промежуточные скаляры для ускорения расчета координат точек вида
  float
    _ca = static_cast<float>(cosf(look_dir[0])),
    _sa = static_cast<float>(sinf(look_dir[0])),
    _ct = static_cast<float>(cosf(look_dir[1]));

  view_mtx.lock();    // смещение камеры за время прошедшее между кадрами
  *ViewFrom.get() += glm::vec3(fb *_ca + rl*sinf(look_dir[0] - Pi), ud,  fb*_sa + rl*_ca);
  view_mtx.unlock();

  ViewTo = *ViewFrom.get() + glm::vec3(_ca*_ct, sinf(look_dir[1]), _sa*_ct); //Направление взгляда

  // Расчет матрицы вида
  MatView = glm::lookAt(*ViewFrom.get(), ViewTo, UpWard);

  // Матрица преобразования
  MatMVP =  MatProjection * MatView;
}


///
/// Рендер 3D пространства сцены в буфер "RenderBuffer"
///
/// \details
/// Так как вершины располагаются группами по 4 шт. (обход: 6 индексов / 2 треугольника ),
/// то можно, по номеру (&vertex_id) любой вершины из группы, используя остаток от деления
/// на число вершин в группе, определить какая по номеру из вершин начинает
/// эту группу и какая заканчивает.( hl_point_id_from, hl_point_id_end )
///
void space_3d::render(void)
{
  if(render_indices.load() < indices_per_face) return;
  calc_position();
  vbo_mtx.lock();

  RenderBuffer->bind();
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
  glEnable(GL_DEPTH_TEST);

  // Построение 3D элементов пространства
  glBindVertexArray(vao_3d);
  Program3d->use();
  for(const auto& A: Program3d->AtribsList) glEnableVertexAttribArray(A.index);
  Program3d->set_uniform("mvp", MatMVP);                      // матрица пространства
  Program3d->set_uniform("light_direction", light_direction); // направление
  Program3d->set_uniform("light_bright", light_bright);       // цвет/яркость
  Program3d->set_uniform("MinId", hl_vertex_id_from);         // начальная вершина подсветки поверхности
  Program3d->set_uniform("MaxId", hl_vertex_id_end);          // последняя вершина подсветки
  glDrawElements(GL_TRIANGLES, render_indices.load(), GL_UNSIGNED_INT, nullptr);
  for(const auto& A: Program3d->AtribsList) glDisableVertexAttribArray(A.index);
  Program3d->unuse();

  glBindVertexArray(0); // vao_3d
  RenderBuffer->unbind();

  uint vertex_id = 0;    // переменная для приема ID вершины из VBO
  RenderBuffer->read_pixel(GLint(xpos), GLint(ypos), &vertex_id);
  vbo_mtx.unlock();

  hl_vertex_id_from = vertex_id - (vertex_id % vertices_per_face);
  hl_vertex_id_end = hl_vertex_id_from + vertices_per_face - 1;

}


///
/// \brief space::resize_event
/// \param width
/// \param height
///
void space_3d::resize_event(int width, int height)
{
  // пересчет матрицы проекции
  auto aspect = static_cast<float>(width) / static_cast<float>(height);
  MatProjection = glm::perspective(fovy, aspect, zNear, zFar);
  RenderBuffer->resize(width, height);
}


///
/// \brief space::cursor_event
/// \param x
/// \param y
///
void space_3d::cursor_event(double x, double y)
{
  // Накапливаем дистанцию смещения мыши между кадрами рендера.
  // Чем больше значение, тем выше скрость поворота камеры.
  cursor_dx += static_cast<float>(x - xpos);
  cursor_dy += static_cast<float>(y - ypos);

  // После получения значения счетчика восстановить позицию курсора
  OGLContext->set_cursor_pos(xpos, ypos);
}


///
/// \brief space::mouse_event
/// \param _button
/// \param _action
/// \param _mods
///
void space_3d::mouse_event(int _button, int _action, int _mods)
{
  mods   = _mods;
  mouse  = _button;
  action = _action;

  if((hl_vertex_id_end/vertices_per_face)>(render_indices/indices_per_face)) return;

  if((mouse == MOUSE_BUTTON_LEFT) && (action == PRESS))
  {
    action = -1;
    click_side_vertex_id.store(hl_vertex_id_end); // добавление вокса
  }

  if((mouse == MOUSE_BUTTON_RIGHT) && (action == PRESS))
  {
    action = -1;
    click_side_vertex_id.store(-hl_vertex_id_end); // удаление вокса
  }
}


///
/// \brief space::keyboard_event
/// \param _key
/// \param _scancode
/// \param _action
/// \param _mods
///
void space_3d::keyboard_event(int _key, int _scancode, int _action, int _mods)
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
/// \brief space::~space
///
space_3d::~space_3d()
{
  render_indices.store(0); // Индикатор для остановки потока загрузки в рендер из БД
  if(nullptr != data_loader) if(data_loader->joinable())
    data_loader->join();   // Ожидание завершения потока
}

} // namespace tr
