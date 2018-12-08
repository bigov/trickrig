//============================================================================
//
// file: space.cpp
//
// Архивная версия файла исходного кода, в которой используется функция
// быстрого поиска пересечения луча и треугольника для нахождения ячейки, на
// которую направлен взгляд из камеры для визульной "подсветки" этой ячейки.
//
//============================================================================
#include "space.hpp"

namespace tr
{

#define EPSILON 0.000001

inline double DOT(double v1[], double v2[])
{
  return (v1[0] * v2[0] + v1[1] * v2[1] + v1[2] * v2[2]);
}

inline void CROSS(double dest[], double v1[], double v2[])
{
  dest[0] = v1[1]*v2[2] - v1[2]*v2[1];
  dest[1] = v1[2]*v2[0] - v1[0]*v2[2];
  dest[2] = v1[0]*v2[1] - v1[1]*v2[0];
  return;
}

inline void SUB(double dest[], double v1[], double v2[])
{
  dest[0] = v1[0] - v2[0];
  dest[1] = v1[1] - v2[1];
  dest[2] = v1[2] - v2[2];
  return;
}

// Быстрый поиск пересечения луча и треугольника описан на странице -
// http://masters.donntu.org/2015/frt/yablokov/library/transl.htm

///
/// находит пересечение луча и треугольника
/// (исх. код: http://masters.donntu.org/2015/frt/yablokov/library/transl.htm)
///
int intersect_triangle(const glm::vec3 &vOrig, const glm::vec3 &vDir,
                       double vert0[3], double vert1[3], double vert2[3])
{
  double
      orig[3] = { vOrig.x, vOrig.y, vOrig.z },
      dir[3] = { vDir.x, vDir.y, vDir.z },
      edge1[3], edge2[3],
      tvec[3], pvec[3], qvec[3],
      det, u, v;

  // найти векторы двух граней, содержащих vert0
  SUB(edge1, vert1, vert0);
  SUB(edge2, vert2, vert0);

  // расчет определителя - также используется для расчета u
  CROSS(pvec, dir, edge2);
  det = DOT(edge1, pvec);

  // если определитель близок к нулю, то луч лежит в плоскости треугольника
  if(det < EPSILON) return 0;

  // расчет расстояния от vert0 до начала луча
  SUB(tvec, orig, vert0);

  // расчет параметра U и проверка границы
  u = DOT(tvec, pvec);
  if(u < 0.0 || u > det) return 0;

  // расчет V и проверка границы
  CROSS(qvec, tvec, edge1);
  v = DOT(dir, qvec);
  if(v < 0.0 || u + v > det) return 0;

  return 1;
}

  //## Формирование 3D пространства
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

    img ImgTex0 { tr::cfg::get(PNG_TEXTURE0) }; // Загрузка из файла текстуры

    glGenTextures(1, &m_textureObj);
    glActiveTexture(GL_TEXTURE0); // можно загрузить не меньше 48
    glBindTexture(GL_TEXTURE_2D, m_textureObj);

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

    RigsDb0.init(g0, Eye.ViewFrom); // начальная загрузка пространства

    MoveFrom = {
      static_cast<int>(floor(static_cast<double>(Eye.ViewFrom.x))),
      static_cast<int>(floor(static_cast<double>(Eye.ViewFrom.y))) - 2,
      static_cast<int>(floor(static_cast<double>(Eye.ViewFrom.z))),
    };

    int // границы уровня lod_0
      xMin = MoveFrom.x - tr::lod_0,
      yMin = MoveFrom.y - tr::lod_0,
      zMin = MoveFrom.z - tr::lod_0,
      xMax = MoveFrom.x + tr::lod_0,
      yMax = MoveFrom.y + tr::lod_0,
      zMax = MoveFrom.z + tr::lod_0;

    // Загрузить в графический буфер атрибуты элементов
    for(int x = xMin; x<= xMax; x += g0)
      for(int y = yMin; y<= yMax; y += g0)
        for(int z = zMin; z<= zMax; z += g0)
          RigsDb0.put_in_vbo(x, y, z);

    try {
      MoveFrom = RigsDb0.search_down(tr::Eye.ViewFrom); // ближайший к камере снизу блок
    }
    catch(...)
    {
      #ifndef NDEBUG
      tr::info("Fail setup 3d coordinates for ViewFrom point.");
      #endif
      //TODO: установить точку обзора над поверхностью
    }

    return;
  }

  //## Построение границы области по оси X по ходу движения
  // TODO: ВНИМАНИЕ! проверяется только 10 слоев по Y
  void space::redraw_borders_x()
  {
    int
      yMin = -5, yMax =  5, // Y границы области сбора
      x_old, x_new,  // координаты линий удаления/вставки новых фрагментов
      vf_x = static_cast<int>(floor(static_cast<double>(Eye.ViewFrom.x))),
      vf_z = static_cast<int>(floor(static_cast<double>(Eye.ViewFrom.z))),
      clod_0 = tr::lod_0;

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
    for(int y = yMin; y <= yMax; y += g0)
      for(int z = zMin; z <= zMax; z += g0)
        RigsDb0.remove_from_vbo(x_old, y, z);

    // Добавить линию элементов по направлению движения
    zMin = vf_z - clod_0;
    zMax = vf_z + clod_0;
    for(int y = yMin; y <= yMax; y += g0)
      for(int z = zMin; z <= zMax; z += g0)
        RigsDb0.put_in_vbo(x_new, y, z);

    MoveFrom.x = vf_x;
    return;
  }

  //## Построение границы области по оси Z по ходу движения
  // TODO: ВНИМАНИЕ! проверяется только 10 слоев по Y
  void space::redraw_borders_z()
  {
    int
      yMin = -5, yMax =  5, // Y границы области сбора
      z_old, z_new,  // координаты линий удаления/вставки новых фрагментов
      vf_z = static_cast<int>(floor(static_cast<double>(Eye.ViewFrom.z))),
      vf_x = static_cast<int>(floor(static_cast<double>(Eye.ViewFrom.x))),
      clod_0 = tr::lod_0;

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
    for(int y = yMin; y <= yMax; y += g0)
      for(int x = xMin; x <= xMax; x += g0)
        RigsDb0.remove_from_vbo(x, y, z_old);

    // Добавить линию элементов по направлению движения
    xMin = vf_x - clod_0;
    xMax = vf_x + clod_0;
    for(int y = yMin; y <= yMax; y += g0)
      for(int x = xMin; x <= xMax; x += g0)
        RigsDb0.put_in_vbo(x, y, z_new);

    MoveFrom.z = vf_z;
    return;
  }

  //## Перестроение границ активной области при перемещении камеры
  void space::recalc_borders(void)
  {
  /* - собираем в кэш адреса удаляемых снипов.
   * - данные добавляемых в сцену снипов записываем в VBO по адресам из кэша.
   * - если кэш пустой, то данные добавляем в конец VBO буфера.
   *
   * TODO? (на случай притормаживания если прыгать камерой туда-сюда
   * через границу запуска перерисовки границ)
   *   Можно процедуры "redraw_borders_?" разбить по две части - отдельно
   *   сбор данных в кэш и отдельно построение новой границы по ходу движения.
   *
   * - запускать их с небольшим зазором (вначале сбор, потом перестроение)
   *
   * - в процедуре построения вначале проверять наличие снипов рига в кэше
   *   и просто активировать их без запроса к базе данных, если они там.
   *
   * Еще можно кэшировать данные в обработчике соединений с базой данных,
   * сохраняя в нем данные, например, о двух линиях одновременно по внешнему
   * периметру. Это должно снизить число обращений к (диску) базе данных при
   * резких "маятниковых" перемещениях камеры туда-сюда.
   */

    if(static_cast<int>(floor(tr::Eye.ViewFrom.x)) != MoveFrom.x) redraw_borders_x();
  //if(static_cast<int>(floor(tr::Eye.ViewFrom.y)) != MoveFrom.y) redraw_borders_y();
    if(static_cast<int>(floor(tr::Eye.ViewFrom.z)) != MoveFrom.z) redraw_borders_z();

    return;
  }

  //## Расчет положения и направления движения камеры
  void space::calc_position(const evInput & ev)
  {
    Eye.look_a -= ev.dx * Eye.look_speed;
    if(Eye.look_a > two_pi) Eye.look_a -= two_pi;
    if(Eye.look_a < 0) Eye.look_a += two_pi;

    Eye.look_t -= ev.dy * Eye.look_speed;
    if(Eye.look_t > look_up) Eye.look_t = look_up;
    if(Eye.look_t < look_down) Eye.look_t = look_down;

    float _k = Eye.speed / static_cast<float>(AppWin.fps); // корректировка по FPS

    //if (!space_is_empty(tr::Eye.ViewFrom)) _k *= 0.1f;       // TODO: скорость/туман в воде

    rl = _k * static_cast<float>(ev.rl);   // скорости движения
    fb = _k * static_cast<float>(ev.fb);   // по трем нормалям от камеры
    ud = _k * static_cast<float>(ev.ud);

    // промежуточные скаляры для ускорения расчета координат точек вида
    float
      _ca = static_cast<float>(cos(Eye.look_a)),
      _sa = static_cast<float>(sin(Eye.look_a)),
      _ct = static_cast<float>(cos(Eye.look_t));

    glm::vec3 LookDir {_ca*_ct, sin(Eye.look_t), _sa*_ct}; //Направление взгляда
    tr::Eye.ViewFrom += glm::vec3(fb *_ca + rl*sin(Eye.look_a - pi), ud,  fb*_sa + rl*_ca);
    ViewTo = tr::Eye.ViewFrom + LookDir;

    // Расчет матрицы вида
    MatView = glm::lookAt(tr::Eye.ViewFrom, ViewTo, UpWard);

    //calc_selected_area(LookDir);
    return;
  }

  ///
  /// Расчет координат рига, на который направлен взгляд
  ///
  void space::calc_selected_area(glm::vec3 & LookDir)
  {
    Selected = { 0, 0, 0 };

    auto search = Eye.ViewFrom;
    glm::vec3 step = LookDir/2.f;

    int x, y, z;

    int i = 0, i_max = 6;
    while(i < i_max)
    {
      x = static_cast<int>(floor(search.x));
      y = static_cast<int>(floor(search.y));
      z = static_cast<int>(floor(search.z));
      if(RigsDb0.get(x,y,z) != nullptr) break;
      search += step;
      ++i;
    }
    if(i == i_max) return;

    auto R = RigsDb0.get(Selected);
    auto S = R->Trick.front();      // верхний снип в найденном риге

    // Если определен риг, в сторону которого направлен взгляд, то проверим -
    // проходит ли через его снип "луч взгляда". Если не проходит - то
    // нам нужен следущий по направлению взгляда.

    //координаты вершин снипа
    double A[3] { S.data[SNIP_ROW_DIGITS * 0 + SNIP_X] + x,
                  S.data[SNIP_ROW_DIGITS * 0 + SNIP_Y] + y,
                  S.data[SNIP_ROW_DIGITS * 0 + SNIP_Z] + z, };
    double B[3] { S.data[SNIP_ROW_DIGITS * 1 + SNIP_X] + x,
                  S.data[SNIP_ROW_DIGITS * 1 + SNIP_Y] + y,
                  S.data[SNIP_ROW_DIGITS * 1 + SNIP_Z] + z, };
    double C[3] { S.data[SNIP_ROW_DIGITS * 2 + SNIP_X] + x,
                  S.data[SNIP_ROW_DIGITS * 2 + SNIP_Y] + y,
                  S.data[SNIP_ROW_DIGITS * 2 + SNIP_Z] + z, };
    double D[3] { S.data[SNIP_ROW_DIGITS * 3 + SNIP_X] + x,
                  S.data[SNIP_ROW_DIGITS * 3 + SNIP_Y] + y,
                  S.data[SNIP_ROW_DIGITS * 3 + SNIP_Z] + z, };

    // Снип состоит из двух треугольников. Проверяем пересечение с одним из них:
    if(intersect_triangle(Eye.ViewFrom, LookDir, A, B, C) ||
       intersect_triangle(Eye.ViewFrom, LookDir, C, D, A) )
    {
       Selected = { x, y, z }; // Если есть пересечение, то поиск закончен
    }
    else
    { // Если луч взгляда не пересекает найденый снип, то берем следущий
      // по направлению взгляда:
      search += LookDir;
      x = static_cast<int>(floor(search.x));
      y = static_cast<int>(floor(search.y));
      z = static_cast<int>(floor(search.z));
      Selected = { x, y, z };
    }
    return;
  }

  ///
  /// Функция, вызываемая из цикла окна для рендера сцены
  ///
  void space::draw(const evInput & ev)
  {
    calc_position(ev);
    recalc_borders();
    //RigsDb0.highlight(Selected); // Подсветка выделения

    // Запись в базу данных: Ctrl+S (285, 31).
    // Код отрабатывает последовательное нажатие клавиш Ctrl(285) и S(31)
    static int mod = 0;
    if((31 == ev.key_scancode) && (1 == mod))
    {
      RigsDb0.save({0, 0, 0}, {16, 1, 16});
    }
    mod = 0;
    if (285 == ev.key_scancode) mod = 1;


    // Матрицу модели в расчетах не используем, так как
    // она единичная и на положение элементов влияние не оказывает
    RigsDb0.draw( MatProjection * MatView );
    return;
  }

} // namespace tr
