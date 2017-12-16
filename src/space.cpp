//============================================================================
//
// file: scene.cpp
//
// Управление пространством 3D сцены
//
//============================================================================
#include "space.hpp"

namespace tr
{
  //## Формирование 3D пространства
  space::space(void)
  {
    RigsDb0.gage = 1.0f; // размер стороны для LOD-0
    db_connect();
    vbo_allocate_mem();

    glClearColor(0.5f, 0.69f, 1.0f, 1.0f);
    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
    glFrontFace(GL_CCW);
    glCullFace(GL_BACK);
    glEnable(GL_CULL_FACE); // после загрузки сцены опция выключается
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_LINE_SMOOTH);
    glEnable(GL_BLEND); // поддержка прозрачности
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    // Загрузка из файла данных текстуры
    pngImg image = get_png_img(tr::Config::filepath(TEXTURE));

    glGenTextures(1, &m_textureObj);
    glActiveTexture(GL_TEXTURE0); // можно загрузить не меньше 48
    glBindTexture(GL_TEXTURE_2D, m_textureObj);

    GLint level_of_details = 0;
    GLint frame = 0;
    glTexImage2D(GL_TEXTURE_2D, level_of_details, GL_RGBA,
      image.w, image.h, frame, GL_RGBA, GL_UNSIGNED_BYTE, image.img.data());

    // Установка опций отрисовки текстур
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
    glGenerateMipmap(GL_TEXTURE_2D);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER,
      GL_NEAREST_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

    upload_vbo();
    return;
  }

  //## Загрузка в VBO (графическую память) данных отображаемых объектов 3D сцены
  void space::upload_vbo(void)
  {
  // TODO: тут должны загружаться в графическую память все lod_?,
  //       но пока загружается только lod_0

    f3d pt = RigsDb0.search_down(ViewFrom); // ближайший к камере снизу блок

    // используется в функциях пересчета границ отрисовки областей
    MoveFrom = {floor(pt.x), floor(pt.y), floor(pt.z)};

    float // границы уровня lod_0
      xMin = MoveFrom.x - ceil(tr::lod_0),
      yMin = MoveFrom.y - ceil(tr::lod_0),
      zMin = MoveFrom.z - ceil(tr::lod_0),
      xMax = MoveFrom.x + ceil(tr::lod_0),
      yMax = MoveFrom.y + ceil(tr::lod_0),
      zMax = MoveFrom.z + ceil(tr::lod_0);

    // Загрузить в графический буфер атрибуты элементов
    for(float x = xMin; x<= xMax; x += RigsDb0.gage)
    for(float y = yMin; y<= yMax; y += RigsDb0.gage)
    for(float z = zMin; z<= zMax; z += RigsDb0.gage)
      if(RigsDb0.exist(x, y, z)) vbo_data_send(x, y, z);

    glDisable(GL_CULL_FACE); // включить отображение обратных поверхностей

    return;
  }

  //## Соединение с БД, в которой хранится виртуальное 3D пространство
  void space::db_connect(void)
  {
  // TODO: должно быть заменено на подключение к базе данных пространства,

    int s = 10;
    int y = 0.f;

    for (int x = 0 - s; x < s; x += 1)
      for (int z = 0 - s; z < s; z += 1)
      {
        RigsDb0.emplace(x, y, z);
      }

    // обозначить центр и оси координат
    RigsDb0.get(0,0,0 )->area.front().texture_set(0.125, 0.125*7);
    RigsDb0.get(1,0,0 )->area.front().texture_set(0.125, 0.0);
    RigsDb0.get(-1,0,0)->area.front().texture_set(0.125, 0.125);
    RigsDb0.get(0,0,1 )->area.front().texture_set(0.125, 0.125*4);
    RigsDb0.get(0,0,-1)->area.front().texture_set(0.125, 0.125*5);

    tr::f3d P = {1.f, 0.f, 1.f};
    // Загрузить объект из внешнего файла
    tr::loader_obj Obj("../assets/test_flat.obj");
    RigsDb0.set(P, Obj.Area);

    return;
  }

  //## запись данных в графический буфер
  void space::vbo_data_send(float x, float y, float z)
  {
  /* Индексы размещенных в VBO данных, которые при перемещении камеры вышли
   * за границу отображения, запоминаются в списке CasheVBOptr, чтобы на их место
   * записать данные точек, которые вошли в поле зрения с другой стороны.
   */

    tr::rig* Rig = RigsDb0.get(x, y, z);
    #ifndef NDEBUG
    if(nullptr == Rig) ERR("ERR: call post_key for empty space.");
    #endif

    glBindVertexArray(space_vao);

    for(auto & Snip: Rig->area)
      if(CashedSnips.empty()) // Если кэш пустой, то добавляем данные в конец VBO
      {
        Snip.vbo_append(VBOdata, VBOindex);
        render_points += tr::indices_per_snip;  // увеличить число точек рендера
        VisibleSnips[Snip.data_offset] = &Snip; // добавить ссылку
      }
      else // если в кэше есть данные, то пишем на их место
      {
        Snip.vbo_update(VBOdata, VBOindex, CashedSnips.front());
        CashedSnips.pop_front();                // укоротить кэш
        VisibleSnips[Snip.data_offset] = &Snip; // добавить ссылку
       }

    glBindVertexArray(0);

    return;
  }

  //## Инициализировать, настроить VBO и заполнить данными
  void space::vbo_allocate_mem(void)
  {
    Prog3d.attach_shaders(
      tr::Config::filepath(VERT_SHADER),
      tr::Config::filepath(FRAG_SHADER)
    );
    Prog3d.use();

    // инициализация VAO
    glGenVertexArrays(1, &space_vao);
    glBindVertexArray(space_vao);

    // Число элементов в кубе с длиной стороны = "space_i0_length" элементов:
    unsigned n = pow((tr::lod_0 + tr::lod_0 + 1), 3);

    // число байт для заполнения такого объема прямоугольниками:
    VBOdata.allocate(n * tr::snip_data_bytes);
    // настройка положения атрибутов
    VBOdata.attrib(Prog3d.attrib_location_get("position"),
      4, GL_FLOAT, GL_FALSE, tr::snip_bytes_per_vertex, (void*)(0 * sizeof(GLfloat)));
    VBOdata.attrib(Prog3d.attrib_location_get("color"),
      4, GL_FLOAT, GL_TRUE, tr::snip_bytes_per_vertex, (void*)(4 * sizeof(GLfloat)));
    VBOdata.attrib(Prog3d.attrib_location_get("normal"),
      4, GL_FLOAT, GL_TRUE, tr::snip_bytes_per_vertex, (void*)(8 * sizeof(GLfloat)));
    VBOdata.attrib(Prog3d.attrib_location_get("fragment"),
      2, GL_FLOAT, GL_TRUE, tr::snip_bytes_per_vertex, (void*)(12 * sizeof(GLfloat)));

    // индексный массив
    VBOindex.allocate(static_cast<size_t>(6 * n * sizeof(GLuint)));

    glBindVertexArray(0);
    Prog3d.unuse();

    return;
  }

  //## Построение границы области по оси X по ходу движения
  // TODO: ВНИМАНИЕ! проверяется только 10 слоев по Y
  void space::redraw_borders_x()
  {
    rig* Rig;
    float
      yMin = -5.f, yMax =  5.f, // Y границы области сбора
      x_old, x_new,  // координаты линий удаления/вставки новых фрагментов
      vf_x = floor(ViewFrom.x), vf_z = floor(ViewFrom.z),
      clod_0 = ceil(tr::lod_0);

    if(MoveFrom.x > vf_x) {
      x_old = MoveFrom.x + clod_0;
      x_new = vf_x - clod_0;
    } else {
      x_old = MoveFrom.x - clod_0;
      x_new = vf_x + clod_0;
    }

    // Сбор индексов VBO с задней границы области
    float zMin = MoveFrom.z - clod_0, zMax = MoveFrom.z + clod_0;
    for(float y = yMin; y <= yMax; y += RigsDb0.gage)
    for(float z = zMin; z <= zMax; z += RigsDb0.gage)
    {
      Rig = RigsDb0.get(x_old, y, z);
      if(nullptr != Rig) for(auto & Snip: Rig->area)
      {
        VisibleSnips.erase(Snip.data_offset);
        CashedSnips.push_front(Snip.data_offset);
      }
    }

    // Построение линии по направлению движения
    zMin = vf_z - clod_0; zMax = vf_z + clod_0;
    for(float y = yMin; y <= yMax; y += RigsDb0.gage)
      for(float z = zMin; z <= zMax; z += RigsDb0.gage)
        if(RigsDb0.exist(x_new, y, z)) vbo_data_send(x_new, y, z);

    MoveFrom.x = vf_x;
    return;
  }

  //## Построение границы области по оси Z по ходу движения
  // TODO: ВНИМАНИЕ! проверяется только 10 слоев по Y
  void space::redraw_borders_z()
  {
    tr::rig* Rig;
    float
      yMin = -5.f, yMax =  5.f, // Y границы области сбора
      z_old, z_new,  // координаты линий удаления/вставки новых фрагментов
      vf_z = floor(ViewFrom.z), vf_x = floor(ViewFrom.x),
      clod_0 = ceil(tr::lod_0);

    if(MoveFrom.z > vf_z) {
      z_old = MoveFrom.z + clod_0;
      z_new = vf_z - clod_0;
    } else {
      z_old = MoveFrom.z - clod_0;
      z_new = vf_z + clod_0;
    }

    // Сбор индексов VBO с задней границы области
    float xMin = MoveFrom.x - clod_0, xMax = MoveFrom.x + clod_0;
    for(float y = yMin; y <= yMax; y += RigsDb0.gage)
    for(float x = xMin; x <= xMax; x += RigsDb0.gage)
    {
      Rig = RigsDb0.get(x, y, z_old);
      if(nullptr != Rig) for(auto & Snip: Rig->area)
      {
        VisibleSnips.erase(Snip.data_offset);
        CashedSnips.push_front(Snip.data_offset);
      }
    }

    // Построение линии по направлению движения
    xMin = vf_x - clod_0; xMax = vf_x + clod_0;
    for(float y = yMin; y <= yMax; y += RigsDb0.gage)
      for(float x = xMin; x <= xMax; x += RigsDb0.gage)
        if(RigsDb0.exist(x, y, z_new)) vbo_data_send(x, y, z_new);

    MoveFrom.z = vf_z;
    return;
  }

  //## Перестроение границ активной области при перемещении камеры
  void space::recalc_borders(void)
  {
  /* - собираем в кэш адреса удаляемых снипов.
   * - добавляемые в сцену снипы пишем в VBO по адресам из кэша.
   * - если кэш пустой, то добавляем в конец буфера.
   */
    if(floor(ViewFrom.x) != MoveFrom.x) redraw_borders_x();
    //if(floor(ViewFrom.y) != MoveFrom.y) redraw_borders_y();
    if(floor(ViewFrom.z) != MoveFrom.z) redraw_borders_z();
    return;
  }

  //## Удаление элементов по адресам с кэше и сжатие данных в VBO
  void space::clear_cashed_snips(void)
  {
  /* Если в кэше есть адреса из середины VBO, то на них переносим данные
   * из конца и сжимаем буфер на один блок. Если адрес в кэше из конца VBO,
   * то сразу сжимаем буфер.
   */

    GLsizeiptr data_src = VBOdata.get_hem(); // граница блока данных VBO
    if(data_src == 0 )
    {
      CashedSnips.clear();
      VisibleSnips.clear();
      render_points = 0;
      return;
    }
    data_src -= tr::snip_data_bytes; // адрес последнего блока

    #ifndef NDEBUG
      if(data_src < 0) ERR ("space::jam_vbo got error address in VBO");
    #endif

    // если этот элемент не в рендере, то сжать буфер
    if(VisibleSnips.find(data_src) == VisibleSnips.end())
    {
      VBOdata.shrink(tr::snip_data_bytes);   // укоротить VBO данных
      VBOindex.shrink(tr::snip_index_bytes); // укоротить VBO индекса
      render_points -= tr::indices_per_snip; // уменьшить число точек рендера
      return;
    }

    // извлечь из кэша адрес
    GLsizeiptr data_dst = CashedSnips.front(); CashedSnips.pop_front();
    if(data_dst >= data_src) return;

    try {
      tr::snip *Snip = VisibleSnips.at(data_src);  // переместить снип
      Snip->jam_data(VBOdata, VBOindex, data_dst);
      VisibleSnips[Snip->data_offset] = Snip;   // обновить ссылку
      render_points -= tr::indices_per_snip; // уменьшить число точек рендера
    } catch (...) {
      ERR("space::jam_vbo got error VisibleSnips[data_src]");
    }

    return;
  }

  //## Расчет положения и направления движения камеры
  void space::calc_position(const evInput & ev)
  {
    look_a += ev.dx * k_mouse;
    if(look_a > two_pi) look_a -= two_pi;
    if(look_a < 0) look_a += two_pi;

    look_t -= ev.dy * k_mouse;
    if(look_t > look_up) look_t = look_up;
    if(look_t < look_down) look_t = look_down;

    float _k = k_sense / static_cast<float>(ev.fps); // корректировка по FPS
    //if (!space_is_empty(ViewFrom)) _k *= 0.1f;  // в воде TODO: добавить туман

    rl = _k * static_cast<float>(ev.rl);   // скорости движения
    fb = _k * static_cast<float>(ev.fb);   // по трем осям
    ud = _k * static_cast<float>(ev.ud);

    // промежуточные скаляры для ускорения расчета координат точек вида
    float
      _ca = static_cast<float>(cos(look_a)),
      _sa = static_cast<float>(sin(look_a)),
      _ct = static_cast<float>(cos(look_t));

    glm::vec3 LookDir {_ca*_ct, sin(look_t), _sa*_ct}; //Направление взгляда
    ViewFrom += glm::vec3(fb *_ca + rl*sin(look_a - pi), ud,  fb*_sa + rl*_ca);
    ViewTo = ViewFrom + LookDir;

    // Расчет матрицы вида
    MatView = glm::lookAt(ViewFrom, ViewTo, UpWard);

    calc_selected_area(LookDir);
    return;
  }

  //## Расчет координат ближнего блока, на который направлен взгляд
  void space::calc_selected_area(glm::vec3 & s_dir)
  {
     Selected = ViewFrom - s_dir;
     return;

   /*               ******** ! отключено ! ********             */

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
  }

  //## Функция, вызываемая из цикла окна для рендера сцены
  void space::draw(const evInput & ev)
  {
   #ifndef NDEBUG
   static bool first_call = true;
   if(first_call) std::cout << "count of render points = " << render_points << "\n";
   first_call = false;
   #endif

    // Матрицу модели в расчетах не используем, так как
    // она единичная и на положение элементов влияние не оказывает

    calc_position(ev);
    recalc_borders();
    if (!CashedSnips.empty()) clear_cashed_snips();

    Prog3d.use();   // включить шейдерную программу
    Prog3d.set_uniform("mvp", MatProjection * MatView);
    //prog3d.set_uniform("Selected", Selected);

    Prog3d.set_uniform("light_direction", glm::vec4(0.2f, 0.9f, 0.5f, 0.0));
    Prog3d.set_uniform("light_bright", glm::vec4(0.5f, 0.5f, 0.5f, 0.0));
  
    glBindVertexArray(space_vao);

    glActiveTexture(GL_TEXTURE0); // можно загрузить не меньше 48
    glBindTexture(GL_TEXTURE_2D, m_textureObj);

    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glEnable(GL_DEPTH_TEST);
    glDrawElements(GL_TRIANGLES, render_points, GL_UNSIGNED_INT, NULL);

    glBindVertexArray(0);
  
    Prog3d.unuse(); // отключить шейдерную программу
    return;
  }

} // namespace tr
