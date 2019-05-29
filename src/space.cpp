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
const double
  hPi = acos(0),                   // половина константы "Пи"
  Pi  = 2 * hPi,                   // константа "Пи"
  dPi = 2 * Pi;                    // двойная "Пи
const float up_max = hPi - 0.001f; // Максимальный угол вверх
const float down_max = -up_max;    // Максимальный угол вниз


///
/// \brief space::~space
///
space::~space(void)
{
  Prog3d = nullptr;
  AppWin.RenderBuffer = nullptr;
}


///
/// \brief space::space
/// \details Формирование 3D пространства
///
space::space(void)
{
  Prog3d = std::make_unique<glsl>();
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
  Prog3d->attach_shaders( cfg::app_key(SHADER_VERT_SCENE), cfg::app_key(SHADER_FRAG_SCENE) );
  Prog3d->use();

  // Заполнить карту атрибутов для более быстрого доступа
  Prog3d->attrib_location_get("position");
  Prog3d->attrib_location_get("color");
  Prog3d->attrib_location_get("normal");
  Prog3d->attrib_location_get("fragment");

  Prog3d->unuse();

  // настройка VAO
  init_vao();

  // загрузка основной текстуры
  load_texture(GL_TEXTURE0, cfg::app_key(PNG_TEXTURE0));

  // настройка рендер-буфера с двумя текстурами

  AppWin.RenderBuffer = std::make_unique<frame_buffer> ();

  if(!AppWin.RenderBuffer->init(AppWin.width, AppWin.height))
    ERR("Error on creating Render Buffer.");
}


///
/// \brief space::init_vao
/// \details инициализация VAO
///
void space::init_vao(void)
{
  glGenVertexArrays(1, &vao_id);
  glBindVertexArray(vao_id);

  // Число элементов в кубе с длиной стороны = "space_i0_length" элементов:
  u_int n = static_cast<u_int>(pow((lod0 + lod0 + 1), 3));

  // Размер данных VBO для размещения снипов:
  VBO.allocate(n * bytes_per_side);

  // настройка положения атрибутов
  VBO.attrib(Prog3d->Atrib["position"],
    3, GL_FLOAT, GL_FALSE, bytes_per_vertex, 0 * sizeof(GLfloat));

  VBO.attrib(Prog3d->Atrib["color"],
    4, GL_FLOAT, GL_TRUE, bytes_per_vertex, 3 * sizeof(GLfloat));

  VBO.attrib(Prog3d->Atrib["normal"],
    3, GL_FLOAT, GL_TRUE, bytes_per_vertex, 7 * sizeof(GLfloat));

  VBO.attrib(Prog3d->Atrib["fragment"],
    2, GL_FLOAT, GL_TRUE, bytes_per_vertex, 10 * sizeof(GLfloat));

  //
  // Так как все четырехугольники в снипах индексируются одинаково, то индексный массив
  // заполняем один раз "под завязку" и забываем про него. Число используемых индексов
  // будет всегда соответствовать числу элементов, передаваемых в процедру "glDraw..."
  //
  size_t idx_size = static_cast<size_t>(6 * n * sizeof(GLuint)); // Размер индексного массива
  GLuint *idx_data = new GLuint[idx_size];                       // данные для заполнения
  GLuint idx[6] = {0, 1, 2, 2, 3, 0};                            // шаблон четырехугольника
  GLuint stride = 0;                                             // число описаных вершин
  for(size_t i = 0; i < idx_size; i += 6) {                        // заполнить массив для VBO
    for(size_t x = 0; x < 6; x++) idx_data[x + i] = idx[x] + stride;
    stride += 4;                                                 // по 4 вершины на снип
  }
  VBOindex.allocate(static_cast<GLsizei>(idx_size), idx_data);   // и заполнить данными.
  delete[] idx_data;                                             // Удалить исходный массив.
  glBindVertexArray(0);
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
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER,
    GL_NEAREST_MIPMAP_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
}


///
/// \brief space::init
///
void space::init3d(void)
{
  // начальная загрузка пространства масштаба g0, в точке Eye.ViewFrom
  VBO.clear();
  Area0.load_space(&VBO);

  MoveFrom = {
    static_cast<int>(floor(static_cast<double>(Eye.ViewFrom.x))),
    static_cast<int>(floor(static_cast<double>(Eye.ViewFrom.y))) - 2,
    static_cast<int>(floor(static_cast<double>(Eye.ViewFrom.z))),
  };

  int // границы уровня lod_0
    xMin = MoveFrom.x - lod0,
    yMin = MoveFrom.y - lod0,
    zMin = MoveFrom.z - lod0,
    xMax = MoveFrom.x + lod0,
    yMax = MoveFrom.y + lod0,
    zMax = MoveFrom.z + lod0;

  // Загрузить в графический буфер элементы пространства
  for(int x = xMin; x<= xMax; x += g1)
    for(int y = yMin; y<= yMax; y += g1)
      for(int z = zMin; z<= zMax; z += g1)
        Area0.voxel_draw(Area0.get({x, y, z}));
}


///
/// \brief space::redraw_borders_x
/// \details Построение границы области по оси X по ходу движения
///
void space::redraw_borders_x()
{
  int
    yMin = -5, yMax =  5, // Y границы области сбора
    x_old,                // координаты линий удаления фрагментов
    x_new,                // координаты линий вставки новых фрагментов
    vf_x = static_cast<int>(floor(static_cast<double>(Eye.ViewFrom.x))),
    vf_z = static_cast<int>(floor(static_cast<double>(Eye.ViewFrom.z)));

  if(MoveFrom.x > vf_x) {
    x_old = MoveFrom.x + lod0;
    x_new = vf_x - lod0;
  } else {
    x_old = MoveFrom.x - lod0;
    x_new = vf_x + lod0;
  }

  int zMin, zMax;

  // Скрыть элементы с задней границы области
  zMin = MoveFrom.z - lod0;
  zMax = MoveFrom.z + lod0;
  for(int y = yMin; y <= yMax; y += g1)
    for(int z = zMin; z <= zMax; z += g1)
      Area0.voxel_wipe(Area0.get({x_old, y, z}));

  // Добавить линию элементов по направлению движения
  zMin = vf_z - lod0;
  zMax = vf_z + lod0;
  for(int y = yMin; y <= yMax; y += g1)
    for(int z = zMin; z <= zMax; z += g1)
      Area0.voxel_draw(Area0.get({x_new, y, z}));

  MoveFrom.x = vf_x;
}


///
/// \brief space::redraw_borders_z
/// \details Построение границы области по оси Z по ходу движения
///
void space::redraw_borders_z()
{
  int
    yMin = -5, yMax =  5, // Y границы области сбора
    z_old, z_new,  // координаты линий удаления/вставки новых фрагментов
    vf_z = static_cast<int>(floor(static_cast<double>(Eye.ViewFrom.z))),
    vf_x = static_cast<int>(floor(static_cast<double>(Eye.ViewFrom.x)));

  if(MoveFrom.z > vf_z) {
    z_old = MoveFrom.z + lod0;
    z_new = vf_z - lod0;
  } else {
    z_old = MoveFrom.z - lod0;
    z_new = vf_z + lod0;
  }

  int xMin, xMax;

  // Скрыть элементы с задней границы области
  xMin = MoveFrom.x - lod0;
  xMax = MoveFrom.x + lod0;
  for(int y = yMin; y <= yMax; y += g1)
    for(int x = xMin; x <= xMax; x += g1)
      Area0.voxel_wipe(Area0.get({x, y, z_old}));

  // Добавить линию элементов по направлению движения
  xMin = vf_x - lod0;
  xMax = vf_x + lod0;
  for(int y = yMin; y <= yMax; y += g1)
    for(int x = xMin; x <= xMax; x += g1)
      Area0.voxel_draw(Area0.get({x, y, z_new}));

  MoveFrom.z = vf_z;
}


///
/// \brief space::recalc_borders
/// \details Перестроение границ активной области при перемещении камеры
///
/// TODO? (на случай притормаживания - если прыгать камерой туда-сюда через
/// границу запуска перерисовки границ) можно процедуры "redraw_borders_?"
/// разбить по две части.
///
/// Еще можно кэшировать данные в обработчике соединений с базой данных,
/// сохраняя в нем данные, например, о двух линиях одновременно по внешнему
/// периметру. Это должно снизить число обращений к (диску) базе данных при
/// резких "маятниковых" перемещениях камеры туда-сюда.
///
void space::recalc_borders(void)
{
  if(static_cast<int>(floor(Eye.ViewFrom.x)) != MoveFrom.x) redraw_borders_x();
//if(static_cast<int>(floor(Eye.ViewFrom.y)) != MoveFrom.y) redraw_borders_y();
  if(static_cast<int>(floor(Eye.ViewFrom.z)) != MoveFrom.z) redraw_borders_z();
}


///
/// \brief space::calc_position
/// \param ev
/// \details Расчет положения и направления движения камеры
///
void space::calc_position(evInput & ev)
{
  Eye.look_a -= ev.dx * Eye.look_speed;
  if(Eye.look_a > dPi) Eye.look_a -= dPi;
  if(Eye.look_a < 0) Eye.look_a += dPi;
  ev.dx = 0.f;

  Eye.look_t -= ev.dy * Eye.look_speed;
  if(Eye.look_t > up_max) Eye.look_t = up_max;
  if(Eye.look_t < down_max) Eye.look_t = down_max;
  ev.dy = 0.f;

  float _k = Eye.speed / static_cast<float>(AppWin.fps); // корректировка по FPS

  //if (!space_is_empty(Eye.ViewFrom)) _k *= 0.1f;       // TODO: скорость/туман в воде

  rl = _k * static_cast<float>(ev.rl);   // скорости движения
  fb = _k * static_cast<float>(ev.fb);   // по трем нормалям от камеры
  ud = _k * static_cast<float>(ev.ud);

  // промежуточные скаляры для ускорения расчета координат точек вида
  float
    _ca = static_cast<float>(cos(Eye.look_a)),
    _sa = static_cast<float>(sin(Eye.look_a)),
    _ct = static_cast<float>(cos(Eye.look_t));

  glm::vec3 LookDir {_ca*_ct, sin(Eye.look_t), _sa*_ct}; //Направление взгляда
  Eye.ViewFrom += glm::vec3(fb *_ca + rl*sin(Eye.look_a - Pi), ud,  fb*_sa + rl*_ca);
  ViewTo = Eye.ViewFrom + LookDir;

  // Расчет матрицы вида
  MatView = glm::lookAt(Eye.ViewFrom, ViewTo, UpWard);

  // Матрица преобразования
  MatMVP =  MatProjection * MatView;
}


///
/// Функция, вызываемая из цикла окна для рендера сцены
///
/// S == 31; C == 46
void space::draw(evInput& ev)
{
  calc_position(ev);
  recalc_borders();

  /*
  if((ev.key == GLFW_KEY_CAPS_LOCK) && (ev.action == RELEASE))
  {
    ev.key = -1;
    ev.action = -1;
    RigsDb0.caps_lock_toggle();
  }
  */

  int vertex_id = 0;
  AppWin.RenderBuffer->read_pixel(AppWin.Cursor.x, AppWin.Cursor.y, &vertex_id);
  id_point_0 = vertex_id - (vertex_id % vertices_per_side);
  id_point_8 = id_point_0 + vertices_per_side - 1;

  if((46 == ev.scancode) && (ev.action == PRESS))
  {
    ev.action = -1;
    ev.scancode = -1;
    std::cout << "ID=" << vertex_id << " ";
  }

  if((ev.mouse == MOUSE_BUTTON_LEFT) && (ev.action == PRESS))
  {
    ev.action = -1;
    Area0.increase(vertex_id);
  }

  if((ev.mouse == MOUSE_BUTTON_RIGHT) && (ev.action == PRESS))
  {
    ev.action = -1;
    Area0.decrease(vertex_id);
  }

  render_3d_space();
}


///
/// Рендер кадра
///
void space::render_3d_space(void)
{
  glBindVertexArray(vao_id);
  AppWin.RenderBuffer->bind();
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
  glEnable(GL_DEPTH_TEST);

  Prog3d->use();   // включить шейдерную программу
  Prog3d->set_uniform("mvp", MatMVP);
  Prog3d->set_uniform("light_direction", light_direction);  // направление
  Prog3d->set_uniform("light_bright", light_bright);        // цвет/яркость

  Prog3d->set_uniform("MinId", id_point_0);                 // начальная вершина активного вокселя
  Prog3d->set_uniform("MaxId", id_point_8);                 // последняя вершина активного вокселя

  glEnableVertexAttribArray(Prog3d->Atrib["position"]);    // положение 3D
  glEnableVertexAttribArray(Prog3d->Atrib["color"]);       // цвет
  glEnableVertexAttribArray(Prog3d->Atrib["normal"]);      // нормаль
  glEnableVertexAttribArray(Prog3d->Atrib["fragment"]);    // текстура

  // Нарисовать все за один проход:
  glDrawElements(GL_TRIANGLES, static_cast<GLsizei>(Area0.render_indices), GL_UNSIGNED_INT, nullptr);
  //glDrawElements(GL_LINES, static_cast<GLsizei>(RigsDb0.render_indices), GL_UNSIGNED_INT, nullptr);

  // Xid содержит порядковый номер от начала VBO для первой вершины рисуемого прямоугольника
  //GLsizei max = (RigsDb0.render_indices / indices_per_quad) * vertices_per_quad;
  //for (GLsizei i = 0; i < max; i += vertices_per_quad)
  //{
  //  Prog3d->set_uniform1ui("Xid", static_cast<unsigned int>(i));
  //  glDrawElementsBaseVertex(GL_TRIANGLES, indices_per_quad, GL_UNSIGNED_INT, nullptr, i);
  //}

  glDisableVertexAttribArray(Prog3d->Atrib["position"]);
  glDisableVertexAttribArray(Prog3d->Atrib["color"]);
  glDisableVertexAttribArray(Prog3d->Atrib["normal"]);
  glDisableVertexAttribArray(Prog3d->Atrib["fragment"]);

  Prog3d->unuse(); // отключить шейдерную программу
  AppWin.RenderBuffer->unbind();
  glBindVertexArray(0);
 }

} // namespace tr
