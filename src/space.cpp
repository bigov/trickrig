//============================================================================
//
// file: space.cpp
//
// Управление виртуальным 3D пространством
//
//============================================================================
#include "space.hpp"

namespace tr
{
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

    // Загрузка из файла данных текстуры
    tr::image ImgTex0 = get_png_img(tr::TrConfig.get(PNG_TEXTURE0));

    tr::Eye.look_a = std::stof(tr::TrConfig.get(LOOK_AZIM));
    tr::Eye.look_t = std::stof(tr::TrConfig.get(LOOK_TANG));

    glGenTextures(1, &m_textureObj);
    glActiveTexture(GL_TEXTURE0); // можно загрузить не меньше 48
    glBindTexture(GL_TEXTURE_2D, m_textureObj);

    GLint level_of_details = 0;
    GLint frame = 0;
    glTexImage2D(GL_TEXTURE_2D, level_of_details, GL_RGBA,
      ImgTex0.w, ImgTex0.h, frame, GL_RGBA, GL_UNSIGNED_BYTE, ImgTex0.Data.data());

    // Установка опций отрисовки текстур
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
    glGenerateMipmap(GL_TEXTURE_2D);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER,
      GL_NEAREST_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

    RigsDb0.init(g0); // загрузка данных уровня LOD-0

    MoveFrom = {
      static_cast<int>(floor(tr::Eye.ViewFrom.x)),
      static_cast<int>(floor(tr::Eye.ViewFrom.y)) - 2,
      static_cast<int>(floor(tr::Eye.ViewFrom.z)),
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
      vf_x = static_cast<int>(floor(tr::Eye.ViewFrom.x)),
      vf_z = static_cast<int>(floor(tr::Eye.ViewFrom.z)),
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
      vf_z = static_cast<int>( floor(tr::Eye.ViewFrom.z) ),
      vf_x = static_cast<int>( floor(tr::Eye.ViewFrom.x) ),
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
    for(float y = yMin; y <= yMax; y += g0)
      for(float x = xMin; x <= xMax; x += g0)
        RigsDb0.remove_from_vbo(x, y, z_old);

    // Добавить линию элементов по направлению движения
    xMin = vf_x - clod_0;
    xMax = vf_x + clod_0;
    for(float y = yMin; y <= yMax; y += g0)
      for(float x = xMin; x <= xMax; x += g0)
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
    Eye.look_a += ev.dx * Eye.look_speed;
    if(Eye.look_a > two_pi) Eye.look_a -= two_pi;
    if(Eye.look_a < 0) Eye.look_a += two_pi;

    Eye.look_t -= ev.dy * Eye.look_speed;
    if(Eye.look_t > look_up) Eye.look_t = look_up;
    if(Eye.look_t < look_down) Eye.look_t = look_down;

    float _k = Eye.speed / static_cast<float>(ev.fps); // корректировка по FPS

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

    calc_selected_area(LookDir);
    return;
  }

  //## Расчет координат рига, на который направлен взгляд
  //void space::calc_selected_area(glm::vec3 & s_dir)
  void space::calc_selected_area(glm::vec3&)
  {
     return;

   /*               ******** ! отключено ! ********

    if(static_cast<int>(s_dir.x)) return;

    Selected = ViewTo;
    glm::vec3 check_step = { s_dir.x/8.f, s_dir.y/8.f, s_dir.z/8.f };
    for(int i = 0; i < 24; ++i)
    {
      if(RigsDb0.exist(Selected.x, Selected.y, Selected.z))
      {
        Selected.x = floor(Selected.x);
        Selected.y = floor(Selected.y);
        Selected.z = floor(Selected.z);
        break;
      }
      Selected += check_step;
    }

    return;
    */
  }

  //## Функция, вызываемая из цикла окна для рендера сцены
  void space::draw(const evInput & ev)
  {
    calc_position(ev);
    recalc_borders();

    /// Запись в базу данных: Ctrl+S (285, 31).
    /// Код отрабатывает последовательное нажатие клавиш Ctrl(285) и S(31)
    static int mod = 0;
    if((31 == ev.key_scancode) && (1 == mod))
    {
      RigsDb0.save({0, 0, 0}, {16, 1, 16});
    }
    mod = 0;
    if (285 == ev.key_scancode) mod = 1;

    // Матрицу модели в расчетах не используем, так как
    // она единичная и на положение элементов влияние не оказывает
    RigsDb0.draw( tr::MatProjection * MatView );
    return;
  }

} // namespace tr
