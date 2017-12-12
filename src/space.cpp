//============================================================================
//
// file: scene.cpp
//
// Управление пространством 3D сцены
//
//============================================================================
#include "space.hpp"
#include <iomanip>

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

  //## Как-бэ, дык типа ента... Да про запас!
  space::~space(void)
  {
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
  // а пока имитируем небольшую базу данных Rigs
    int s = 10;
    int y = 0.f;

    for (int x = 0 - s; x < s; x += 1)
      for (int z = 0 - s; z < s; z += 1)
      {
        RigsDb0.emplace(x, y, z);
      }

    //RigsDb0.emplace(0, 2, 0);
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
      if(CasheVBOptr.empty()) // Если кэш пустой, то добавляем данные в конец VBO
      {
        Snip.vbo_append(VBOsurf, VBOidx);
        render_points += tr::indices_per_snip;  // увеличить число точек рендера
        VisibleSnips[Snip.data_offset] = &Snip; // добавить ссылку
      }
      else // если в кэше есть данные, то пишем на их место
      {
        Snip.vbo_update(VBOsurf, VBOidx, CasheVBOptr.front());
        CasheVBOptr.pop_front();                // укоротить кэш
        VisibleSnips[Snip.data_offset] = &Snip; // обновить ссылку
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
    VBOsurf.allocate(n * tr::snip_data_bytes);
    // настройка положения атрибутов
    VBOsurf.attrib(Prog3d.attrib_location_get("position"),
      4, GL_FLOAT, GL_FALSE, tr::snip_bytes_per_vertex, (void*)(0 * sizeof(GLfloat)));
    VBOsurf.attrib(Prog3d.attrib_location_get("color"),
      4, GL_FLOAT, GL_TRUE, tr::snip_bytes_per_vertex, (void*)(4 * sizeof(GLfloat)));
    VBOsurf.attrib(Prog3d.attrib_location_get("normal"),
      4, GL_FLOAT, GL_TRUE, tr::snip_bytes_per_vertex, (void*)(8 * sizeof(GLfloat)));
    VBOsurf.attrib(Prog3d.attrib_location_get("fragment"),
      2, GL_FLOAT, GL_TRUE, tr::snip_bytes_per_vertex, (void*)(12 * sizeof(GLfloat)));

    // индексный массив
    VBOidx.allocate(static_cast<size_t>(6 * n * sizeof(GLuint)));

    glBindVertexArray(0);
    Prog3d.unuse();

    return;
  }

  //## Построение границы области по оси X по ходу движения
  // TODO: ВНИМАНИЕ! проверяется только 10 слоев по Y
  void space::redraw_borders_x()
  {
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
    // Z границы области сбора
    float zMin = MoveFrom.z - clod_0, zMax = MoveFrom.z + clod_0;
    rig* Rig;
    // Сбор индексов VBO по оси X с удаляемой линии границы области
    for(float y = yMin; y <= yMax; y += RigsDb0.gage)
    for(float z = zMin; z <= zMax; z += RigsDb0.gage)
    {
      Rig = RigsDb0.get(x_old, y, z);
      if(nullptr != Rig)
        for(auto & Snip: Rig->area) CasheVBOptr.push_front(Snip.data_offset);
    }
    // Z границы области добавления
    zMin = vf_z - clod_0; zMax = vf_z + clod_0;
    // Построение линии по оси X по направлению движения
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
    // X границы области сбора
    float xMin = MoveFrom.x - clod_0, xMax = MoveFrom.x + clod_0;
    tr::rig* Rig;
    // Сбор индексов VBO по оси Z с удаляемой линии границы области
    for(float y = yMin; y <= yMax; y += RigsDb0.gage)
    for(float x = xMin; x <= xMax; x += RigsDb0.gage)
    {
      Rig = RigsDb0.get(x, y, z_old);
      if(nullptr != Rig)
        for(auto & Snip: Rig->area) CasheVBOptr.push_front(Snip.data_offset);
    }
    // X границы области добавления
    xMin = vf_x - clod_0; xMax = vf_x + clod_0;
    // Построение линии по оси Z по направлению движения
    for(float y = yMin; y <= yMax; y += RigsDb0.gage)
      for(float x = xMin; x <= xMax; x += RigsDb0.gage)
        if(RigsDb0.exist(x, y, z_new)) vbo_data_send(x, y, z_new);

    MoveFrom.z = vf_z;
    return;
  }

  //## Перестроение границ активной области при перемещении камеры
  void space::recalc_borders(void)
  {
  /* - cобираем в кэш список адресов VBO в которых расположены
   *   атрибуты элементов, вышедших за границу отображения.
   *
   * - атрибуты новых элементов, которые следует добавить в сцену, заносим
   *   в VBO по адресам из кэша, перезаписывая их данные.
   *
   * - если новых элементов оказалось больше, то дописываем их в конец буфера.
   *
   * - если меньше, то пока весь кэш не очистится, на освободившееся место
   *   переносим данные из конца буфера и сдвигаем границу отображения, отсекая
   *   отрисовку лишних элементов
   */

    if(floor(ViewFrom.x) != MoveFrom.x) redraw_borders_x();
    //if(floor(ViewFrom.y) != MoveFrom.y) redraw_borders_y();
    if(floor(ViewFrom.z) != MoveFrom.z) redraw_borders_z();

    return;
  }

  //## "Утряска" буферов VBO - удаление элементов по списку cashe_vbo_ptr
  void space::reduce_vbo(void)
  {
  /* Агоритм работы:
   *
   * из VBO адрес крайнего с хвоста блока параметров проверяем, есть ли
   * такой адрес в кэше адресов снятых с рендера элементов (CasheVBOptr). Если
   * есть, то обезаем хвост на один блок, удаляем найденый адрес из кэша и
   * уменьшаем число точек рендера.
   *
   * Если такого адреса а кэше нет, то перемещаем блок на один из свободных
   * адресов из CasheVBOptr, и далее по плану - обезаем хвост, удаляем
   * использованый адрес и уменьшаем число точек.
   *
   * Процедура повторяется каждый кадр, пока в CasheVBOptr есть элементы.
   */

    //DEBUG
    GLsizeiptr dbg_cashe[3] = {0,0,0}; size_t i = 0;
    for(GLsizeiptr _ca: CasheVBOptr) dbg_cashe[i++] = _ca;
    std::cout << dbg_cashe;

    // адрес крайнего в хвоста VBO блока данных
    GLsizeiptr data_src = VBOsurf.get_hem() - tr::snip_data_bytes;

    for(GLsizeiptr cashe_ptr: CasheVBOptr) if(data_src == cashe_ptr)
    {
      VBOsurf.shrink(tr::snip_data_bytes);    // обрезать хвост VBO данных
      VBOidx.shrink(tr::snip_index_bytes);    // обрезать хвост VBO индекса
      VisibleSnips.erase(cashe_ptr);          // удалить из списка рендера
      render_points -= tr::indices_per_snip;  // уменьшить число точек рендера
      CasheVBOptr.remove(cashe_ptr);          // удалить их кэша
      return;
    }

    GLsizeiptr data_dst = CasheVBOptr.front();
    CasheVBOptr.pop_front();                  // извлечь адрес с удалением записи

    #ifndef NDEBUG
      if(VisibleSnips.find(data_src) ==
         VisibleSnips.end()) ERR("space::reduce_vbo - not found map element");
    #endif

    tr::snip *Snip = VisibleSnips[data_src];  // переместить снип
    Snip->jam_data(VBOsurf, VBOidx, data_dst);
    VisibleSnips[Snip->data_offset] = Snip;   // обновить ссылку

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
    if (!CasheVBOptr.empty()) reduce_vbo(); // Очистка неиспользованного кэша

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
