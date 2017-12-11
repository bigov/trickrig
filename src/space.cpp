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
  Space::Space(void)
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
  Space::~Space(void)
  {
    return;
  }

  //## Загрузка в VBO (графическую память) данных отображаемых объектов 3D сцены
  void Space::upload_vbo(void)
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
  void Space::db_connect(void)
  {
  // TODO: должно быть заменено на подключение к базе данных пространства,
  // а пока имитируем небольшую (100х100) базу данных структурой "rigs_db"
    int s = 50;
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
  void Space::vbo_data_send(float x, float y, float z)
  {
  /* Индексы размещенных в VBO данных, которые при перемещении камеры вышли
   * за границу отображения, запоминаются в кэше (idx_ref), чтобы на их место
   * записать данные точек, которые вошли в поле зрения с другой стороны.
   *
   * Входные параметры вершинного шейдера:
   *
   *    vec4 position - 3D координаты вершины
   *    vec4 color    - цвет вершины
   *    vec4 normal   - нормаль вершины
   *    vec2 fragment - 2D коодината в текстурной карте
   */

    tr::rig* r = RigsDb0.get(x, y, z);
    #ifndef NDEBUG
    if(nullptr == r) ERR("ERR: call post_key for empty space.");
    #endif

    glBindVertexArray(space_vao);

    for(auto & fragment: r->area)
      if(cashe_vbo_ptr.empty()) // Если кэше пустой, то добавляем данные в конец VBO
      {
        fragment.vbo_append(VBOsurf, VBOsurfIdx);
        render_points += tr::indices_per_snip;  // увеличить число точек рендера
        visible_rigs[fragment.data_offset] = &fragment;
      }
      else // если же в кэше есть свободная пара индексов, то заменяем данные
      {
        fragment.vbo_update(VBOsurf, VBOsurfIdx, cashe_vbo_ptr.front());
        cashe_vbo_ptr.pop_front();
        visible_rigs[fragment.data_offset] = &fragment;
       }

    glBindVertexArray(0);

    return;
  }

  //## Инициализировать, настроить VBO и заполнить данными
  void Space::vbo_allocate_mem(void)
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
    VBOsurf.Allocate(n * tr::snip_data_size);
    // настройка положения атрибутов
    VBOsurf.Attrib(Prog3d.attrib_location_get("position"),
      4, GL_FLOAT, GL_FALSE, tr::snip_vertex_size, (void*)(0 * sizeof(GLfloat)));
    VBOsurf.Attrib(Prog3d.attrib_location_get("color"),
      4, GL_FLOAT, GL_TRUE, tr::snip_vertex_size, (void*)(4 * sizeof(GLfloat)));
    VBOsurf.Attrib(Prog3d.attrib_location_get("normal"),
      4, GL_FLOAT, GL_TRUE, tr::snip_vertex_size, (void*)(8 * sizeof(GLfloat)));
    VBOsurf.Attrib(Prog3d.attrib_location_get("fragment"),
      2, GL_FLOAT, GL_TRUE, tr::snip_vertex_size, (void*)(12 * sizeof(GLfloat)));

    // индексный массив
    VBOsurfIdx.Allocate(static_cast<size_t>(6 * n * sizeof(GLuint)));

    glBindVertexArray(0);
    Prog3d.unuse();

    return;
  }

  //## Построение границы области по оси X по ходу движения
  // TODO: ВНИМАНИЕ! проверяется только 10 слоев по Y
  void Space::redraw_borders_x()
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
    rig* r;
    // Сбор индексов VBO по оси X с удаляемой линии границы области
    for(float y = yMin; y <= yMax; y += RigsDb0.gage)
    for(float z = zMin; z <= zMax; z += RigsDb0.gage)
    {
      r = RigsDb0.get(x_old, y, z);
      if(nullptr != r)
        for(auto & fr: r->area) cashe_vbo_ptr.push_front(
          std::make_pair(fr.data_offset, fr.idx_offset));
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
  void Space::redraw_borders_z()
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
    rig* r;
    // Сбор индексов VBO по оси Z с удаляемой линии границы области
    for(float y = yMin; y <= yMax; y += RigsDb0.gage)
    for(float x = xMin; x <= xMax; x += RigsDb0.gage)
    {
      r = RigsDb0.get(x, y, z_old);
      if(nullptr != r)
        for(auto & fr: r->area) cashe_vbo_ptr.push_front(
          std::make_pair(fr.data_offset, fr.idx_offset));
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
  void Space::recalc_borders(void)
  {
  /*   Вызвается покадрово.
   *
   * - cобираем в кэш список адресов VBO в которых расположены
   *   атрибуты элементов, вышедших за границу отображения.
   *
   * - атрибуты новых элементов, которые следует добавить в сцену, заносим
   *   в VBO на места старых, перезаписывая их данные.
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


    // Очистка неиспользованных элементов
    if(!cashe_vbo_ptr.empty()) reduce_keys();
    return;
  }

  //## Покадровое "сжатие" буферов при наличии данных в cashe_vbo_ptr
  void Space::reduce_keys(void)
  {
  /* Когда после перемещения камеры в буферах остаются неспользуемые
   * блоки, адреса которых хранятся в списке cashe_vbo_ptr, на их
   * место перемещаем активные из хвоста буфера */

    auto cashe = cashe_vbo_ptr.front(); // берем пару свободных адресов

    // Выбираем адрес крайнего блока индексов
    GLsizeiptr data_src = VBOsurf.get_hem() - tr::snip_data_size;
    GLsizeiptr idx_src = VBOsurfIdx.get_hem() - tr::snip_index_size;

    // Если он совпал с адресом в кэше, то сжимаем границы буферов
    // на один блок, уменьшаем кэш и удаляем запись из массива "visible_rigs"
    if ( idx_src == cashe.second )
    {
       VBOsurf.shrink(tr::snip_data_size);
       VBOsurfIdx.shrink(tr::snip_index_size);
       cashe_vbo_ptr.pop_front(); // удаляем запись из кэша

       auto it = visible_rigs.find(cashe.first);
       if (it != visible_rigs.end()) visible_rigs.erase(it);

       return;
     }

    // если нет, то переносим данные из хвостов буферов на адреса из кэша
    VBOsurf.Reduce(data_src, cashe.first, tr::snip_data_size);
    VBOsurfIdx.Reduce(idx_src, cashe.second, tr::snip_index_size);

    VBOsurf.shrink(tr::snip_data_size); // подрезаем хвосты на длину блоков данных
    VBOsurfIdx.shrink(tr::snip_index_size);
    cashe_vbo_ptr.pop_front();          // удаляем запись из кэша

    // найти адрес фрагмента поверхности, данные которого были перемещены
    auto it = visible_rigs.find(data_src);
    if (it == visible_rigs.end())  ERR("Failure select visible_rigs[id_ref_Source]");

    auto fr = visible_rigs[data_src];  // получить адрес объекта фрагмента поверхности
    fr->data_offset = cashe.first;     // заменить в нем адрес буфера данных
    fr->idx_offset = cashe.second;     // заменить в нем адрес буфера индексов
    visible_rigs[cashe.first] = fr;    // обновить ссылку в массиве "visible_rigs"
    visible_rigs.erase(it);            // удалить ссылку со старого адреса

    return;
  }

  //## Расчет положения и направления движения камеры
  void Space::calc_position(const evInput & ev)
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
    MatView = glm::lookAt(ViewFrom, ViewTo, upward);

    calc_selected_area(LookDir);
    return;
  }

  //## Расчет координат ближнего блока, на который направлен взгляд
  void Space::calc_selected_area(glm::vec3 & s_dir)
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
  void Space::draw(const evInput & ev)
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
