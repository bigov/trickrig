//============================================================================
//
// file: space.cpp
//
// Управление виртуальным 3D пространством
//
//============================================================================

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


/// Вычисление расположения отрезка (a, b) относительно (слева или справа)
/// точки c нулевыми координатами на плоскости (x,y)
///
inline bool dir(const glm::vec4 &a, const glm::vec4 &b)
{
  return ((0.0f - b.x) * (a.y - b.y) - (a.x - b.x) * (0.0f - b.y)) > 0.0f;
}


/// \brief Проверка размещения четырехугольника в центре плоскости камеры
/// \return
///
/// Выбор вершин производится в направлении против часовой стрелки
///
inline bool is_target(glm::vec4 &a, glm::vec4 &b, glm::vec4 &c, glm::vec4 &d)
{
  return dir(a, b) && dir(b, c) && dir(c, d) && dir(d, a);
}


///
/// \brief space::space ## Формирование 3D пространства
///
space::space(void)
{
  glClearColor(0.5f, 0.69f, 1.0f, 1.0f);
  glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
  glFrontFace(GL_CCW);
  glCullFace(GL_BACK);
  glDisable(GL_CULL_FACE); // включить отображение обратных поверхностей
  glEnable(GL_DEPTH_TEST);
  glEnable(GL_LINE_SMOOTH);
  glEnable(GL_BLEND);      // поддержка прозрачности
  glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
  load_texture(GL_TEXTURE0, cfg::app_key(PNG_TEXTURE0));
  init_prog3d();
}


///
/// \brief rdb::init_vbo
///
void space::init_prog3d(void)
{
  Prog3d.attach_shaders( cfg::app_key(SHADER_VERT_SCENE), cfg::app_key(SHADER_FRAG_SCENE) );
  Prog3d.use();  // слинковать шейдерную рограмму
  init_vao();    // настроить VAO
  Prog3d.unuse();
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
  u_int n = static_cast<u_int>(pow((lod0_size + lod0_size + 1), 3));

  // Размер данных VBO для размещения снипов:
  VBO.allocate(n * bytes_per_snip);

  // настройка положения атрибутов
  VBO.attrib(Prog3d.attrib_location_get("position"),
    4, GL_FLOAT, GL_FALSE, bytes_per_vertex, 0 * sizeof(GLfloat));

  VBO.attrib(Prog3d.attrib_location_get("color"),
    4, GL_FLOAT, GL_TRUE, bytes_per_vertex, 4 * sizeof(GLfloat));

  VBO.attrib(Prog3d.attrib_location_get("normal"),
    4, GL_FLOAT, GL_TRUE, bytes_per_vertex, 8 * sizeof(GLfloat));

  VBO.attrib(Prog3d.attrib_location_get("fragment"),
    2, GL_FLOAT, GL_TRUE, bytes_per_vertex, 12 * sizeof(GLfloat));

  //
  // Так как все четырехугольники в снипах индексируются одинаково, то индексный массив
  // заполняем один раз "под завязку" и забываем про него. Число используемых индексов
  // будет всегда соответствовать числу элементов, передаваемых в процедру "glDraw..."
  //
  size_t idx_size = static_cast<size_t>(6 * n * sizeof(GLuint)); // Размер индексного массива
  GLuint *idx_data = new GLuint[idx_size];                       // данные для заполнения
  GLuint idx[6] = {0, 1, 2, 2, 3, 0};                            // шаблон четырехугольника
  GLuint stride = 0;                                             // число описаных вершин
  for(size_t i=0; i < idx_size; i += 6) {                        // заполнить массив для VBO
    for(size_t x = 0; x < 6; x++) idx_data[x + i] = idx[x] + stride;
    stride += 4;                                                 // по 4 вершины на снип
  }
  vbo_base VBOindex = { GL_ELEMENT_ARRAY_BUFFER };               // Создать индексный буфер
  VBOindex.allocate(static_cast<GLsizei>(idx_size), idx_data);   // и заполнить данными.
  delete[] idx_data;                                             // Удалить исходный массив.
  glBindVertexArray(0);
}


///
/// \brief space::load_texture
/// \param index
/// \param fname
///
void space::load_texture(unsigned texture_index, const std::string& FileName)
{
  img ImgTex0 { FileName };
  glGenTextures(1, &texture_id);
  glActiveTexture(texture_index); // можно загрузить не меньше 48
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
  RigsDb0.load_space(&VBO, g1, Eye.ViewFrom);

  MoveFrom = {
    static_cast<int>(floor(static_cast<double>(Eye.ViewFrom.x))),
    static_cast<int>(floor(static_cast<double>(Eye.ViewFrom.y))) - 2,
    static_cast<int>(floor(static_cast<double>(Eye.ViewFrom.z))),
  };

  int // границы уровня lod_0
    xMin = MoveFrom.x - lod0_size,
    yMin = MoveFrom.y - lod0_size,
    zMin = MoveFrom.z - lod0_size,
    xMax = MoveFrom.x + lod0_size,
    yMax = MoveFrom.y + lod0_size,
    zMax = MoveFrom.z + lod0_size;

  // Загрузить в графический буфер элементы пространства
  for(int x = xMin; x<= xMax; x += g1)
    for(int y = yMin; y<= yMax; y += g1)
      for(int z = zMin; z<= zMax; z += g1)
        RigsDb0.rig_place(RigsDb0.get({x, y, z}));

  try {
    MoveFrom = RigsDb0.search_down(Eye.ViewFrom); // ближайший к камере снизу блок
  }
  catch(...)
  {
    //TODO: установить точку обзора над поверхностью
    tr::info("Fail setup 3d coordinates for ViewFrom point.");
  }
}


///
/// \brief space::redraw_borders_x
/// \details Построение границы области по оси X по ходу движения
///
void space::redraw_borders_x()
{
  int
    yMin = -5, yMax =  5, // Y границы области сбора
    x_old, x_new,  // координаты линий удаления/вставки новых фрагментов
    vf_x = static_cast<int>(floor(static_cast<double>(Eye.ViewFrom.x))),
    vf_z = static_cast<int>(floor(static_cast<double>(Eye.ViewFrom.z))),
    clod_0 = lod0_size;

  if(MoveFrom.x > vf_x) {
    x_old = MoveFrom.x + clod_0;
    x_new = vf_x - clod_0;
  } else {
    x_old = MoveFrom.x - clod_0;
    x_new = vf_x + clod_0;
  }

  int zMin, zMax;

  // Скрыть элементы с задней границы области
  zMin = MoveFrom.z - clod_0;
  zMax = MoveFrom.z + clod_0;
  for(int y = yMin; y <= yMax; y += g1)
    for(int z = zMin; z <= zMax; z += g1)
      RigsDb0.rig_remove(RigsDb0.get({x_old, y, z}));

  // Добавить линию элементов по направлению движения
  zMin = vf_z - clod_0;
  zMax = vf_z + clod_0;
  for(int y = yMin; y <= yMax; y += g1)
    for(int z = zMin; z <= zMax; z += g1)
      RigsDb0.rig_place(RigsDb0.get({x_new, y, z}));

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
    vf_x = static_cast<int>(floor(static_cast<double>(Eye.ViewFrom.x))),
    clod_0 = lod0_size;

  if(MoveFrom.z > vf_z) {
    z_old = MoveFrom.z + clod_0;
    z_new = vf_z - clod_0;
  } else {
    z_old = MoveFrom.z - clod_0;
    z_new = vf_z + clod_0;
  }

  int xMin, xMax;

  // Скрыть элементы с задней границы области
  xMin = MoveFrom.x - clod_0;
  xMax = MoveFrom.x + clod_0;
  for(int y = yMin; y <= yMax; y += g1)
    for(int x = xMin; x <= xMax; x += g1)
      RigsDb0.rig_remove(RigsDb0.get({x, y, z_old}));

  // Добавить линию элементов по направлению движения
  xMin = vf_x - clod_0;
  xMax = vf_x + clod_0;
  for(int y = yMin; y <= yMax; y += g1)
    for(int x = xMin; x <= xMax; x += g1)
      RigsDb0.rig_place(RigsDb0.get({x, y, z_new}));

  MoveFrom.z = vf_z;
}


///
/// \brief space::recalc_borders
/// \details Перестроение границ активной области при перемещении камеры
///
/// - собираем в кэш адреса удаляемых снипов. данные добавляемых в сцену
///   снипов записываем в VBO по адресам из кэша.
/// - если кэш пустой, то данные добавляем в конец VBO буфера.
///
/// TODO? (на случай притормаживания - если прыгать камерой туда-сюда через
/// границу запуска перерисовки границ) можно процедуры "redraw_borders_?"
/// разбить по две части - отдельно сбор данных в кэш и отдельно построение
/// новой границы по ходу движения.
///
/// - запускать их с небольшим зазором (вначале сбор, потом перестроение)
/// - в процедуре построения вначале проверять наличие снипов рига в кэше
///   и просто активировать их без запроса к базе данных, если они там.
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

  calc_selected_area(LookDir);
}


///
/// Расчет координат рига, на который направлен взгляд
///
void space::calc_selected_area(glm::vec3 & LookDir)
{
  Selected = { 0, 0, 0 };                        // координаты выбранного рига
  glm::vec3 step = glm::normalize(LookDir)/1.2f; // длина шага поиска
  int i_max = 4;                                 // количество шагов проверки

  glm::vec3 check = Eye.ViewFrom;                // переменная поиска
  tr::rig* R = nullptr;
  for(int i = 0; i < i_max; ++i)
  {
    R = RigsDb0.get(check);
    if(nullptr != R) break;
    check += step;
  }
  if(nullptr == R) return;                       // если пересечение не найдено - выход

  // Проверяем верхний снип в найденном риге
  auto S = R->SideYp.front();
  glm::vec4 mv { R->Origin.x, R->Origin.y, R->Origin.z, 0 };

  // Проекции точек снипа на плоскость камеры
  glm::vec4 A = MatView * (S.vertex_coord(0) + mv);
  glm::vec4 B = MatView * (S.vertex_coord(1) + mv);
  glm::vec4 C = MatView * (S.vertex_coord(2) + mv);
  glm::vec4 D = MatView * (S.vertex_coord(3) + mv);

  if(is_target(A, B, C, D))
  {
    Selected = R->Origin;
  }
  else
  { // Если луч взгляда не пересекает найденый снип, то берем следущий
    // по направлению взгляда:
    check += step;
    R = RigsDb0.get(check);
    if(nullptr != R) Selected = R->Origin;
  }
}


///
/// Функция, вызываемая из цикла окна для рендера сцены
///
void space::draw(evInput & ev)
{
  calc_position(ev);
  recalc_borders();

  if((ev.mouse == MOUSE_BUTTON_LEFT) && (ev.action == PRESS))
  {
    ev.mouse = -1; ev.action = -1;
    RigsDb0.add_y(Selected); // Вставить элемент поверхности над выделенной точкой.
  }
  else if((ev.mouse == MOUSE_BUTTON_RIGHT) && (ev.action == PRESS))
  {
    ev.mouse = -1; ev.action = -1;
    RigsDb0.sub_y(Selected); // Удалить элемент поверхности над выделенной точкой.
  }

  RigsDb0.highlight(Selected); // Подсветка выделения

  // Запись в базу данных: Ctrl+S (285, 31).
  // Код отрабатывает последовательное нажатие клавиш Ctrl(285) и S(31)
  static int mod = 0;
  if((31 == ev.scancode) && (1 == mod))
  {
    cfg::DataBase.save_rigs_block({0, 0, 0}, {16, 1, 16}, RigsDb0);
    //RigsDb0.save({0, 0, 0}, {16, 1, 16});
  }
  mod = 0;
  if (285 == ev.scancode) mod = 1;

  render_3d_space();
}


///
/// Рендер кадра
///
void space::render_3d_space(void)
{
  Prog3d.use();   // включить шейдерную программу
  Prog3d.set_uniform("mvp", MatMVP);
  Prog3d.set_uniform("light_direction", glm::vec4(0.2f, 0.9f, 0.5f, 0.0));
  Prog3d.set_uniform("light_bright", glm::vec4(0.5f, 0.5f, 0.5f, 0.0));

  glBindVertexArray(vao_id);
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
  glEnable(GL_DEPTH_TEST);

  // можно все нарисовать за один проход
  glDrawElements(GL_TRIANGLES, static_cast<GLsizei>(RigsDb0.render_points), GL_UNSIGNED_INT, nullptr);

  // а можно, если потребуется, то пошагово по ячейкам -
  //GLsizei max = (render_points / indices_per_snip) * vertices_per_snip;
  //for (GLsizei i = 0; i < max; i += vertices_per_snip)
  //{
  //  glDrawElementsBaseVertex(GL_TRIANGLES, indices_per_snip, GL_UNSIGNED_INT,
  //      nullptr, i);
  //}

  glBindVertexArray(0);
  Prog3d.unuse(); // отключить шейдерную программу
}

} // namespace tr
