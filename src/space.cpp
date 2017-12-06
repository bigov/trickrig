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
  Space::Space(void)
  {
    rigs_db0.gage = 1.0f; // размер стороны для LOD-0
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

  //## Как-бы, дык типа ента...
  Space::~Space(void)
  {
    delete [] visible_rigs;
    return;
  }

  //## Загрузка в VBO (графическую память) данных отображаемых объектов 3D сцены
  void Space::upload_vbo(void)
  {
  // TODO: тут должны загружаться в графическую память все части LOD,
  //       но пока загружается только нулевой уровень

    f3d pt = rigs_db0.search_down(ViewFrom); // ближайший к камере снизу блок
    MoveFrom = {pt.x, pt.y, pt.z};

    float // границы нулевого уровеня LOD
      xMax = pt.x + space_f0_radius,
      yMax = pt.y + space_f0_radius,
      zMax = pt.z + space_f0_radius,
      xMin = pt.x - space_f0_radius,
      yMin = pt.y - space_f0_radius,
      zMin = pt.z - space_f0_radius;

    // Загрузить в графический буфер атрибуты элементов
    for(float x = xMin; x<= xMax; x += rigs_db0.gage)
    for(float y = yMin; y<= yMax; y += rigs_db0.gage)
    for(float z = zMin; z<= zMax; z += rigs_db0.gage)
      if(rigs_db0.exist(x, y, z)) vbo_data_send(x, y, z);

    glDisable(GL_CULL_FACE); // включить отображение обратных поверхностей

    return;
  }

  //## Соединение с БД, в которой хранится виртуальное 3D пространство
  void Space::db_connect(void)
  {
  // TODO: должно быть заменено на подключение к базе данных пространства,
  // а пока имитируем небольшую (100х100) базу данных структурой "rigs_db"
    float s = 50.f;
    float y = 0.f;
    short block_type = 1;

    for (float x = 0.f - s; x < s; x += 1.f)
      for (float z = 0.f - s; z < s; z += 1.f)
        rigs_db0.emplace(x, y, z, block_type);
    return;
  }

  //## запись данных в графический буфер
  void Space::vbo_data_send(float x, float y, float z)
  {
    // Индексы размещенных в VBO данных, которые при перемещении камеры вышли
    // за границу отображения, запоминаются в кэше (idx_ref), чтобы на их место
    // записать данные точек, которые вошли в поле зрения с другой стороны.

    glBindVertexArray(vao_3d);

    GLfloat data[] = { x, y, z, 0.0f, 1.0f, 0.0f };
    auto sizeof_data = sizeof(data);
    GLsizeiptr offset;

    // Если в кэше нет индексов, то размещаем данные в конце VBO
    if(cashe_vbo_ptr.empty())
    {
      offset = VBO_Inst.SubDataAppend(sizeof_data, data);
      ++count;
    }
    else // если в кэше есть свободный индекс блока (вышеднего за границу
    {    // отображения), то меняем данные по месту его размещения в VBO
      offset = cashe_vbo_ptr.front();
      cashe_vbo_ptr.pop_front();
      VBO_Inst.SubDataUpdate(sizeof_data, data, offset);
    }
    glBindVertexArray(0);

    auto r = rigs_db0.get(x, y, z);
    #ifndef NDEBUG
    if(nullptr == r) ERR("ERR: call post_key for empty space.");
    #endif

    // Запишем в Риг адрес смещения его данных в VBO. Это значение
    // потребуется при замене блока данных после выхода за границу
    // отображаемой области.
    r->vbo_offset = offset;

    // порядковым номером блока данных в VBO индексируем массив ссылок, в котором
    // храним адреса расположеных там Rig-ов
    visible_rigs[static_cast<size_t>(offset/sizeof_data)] = r;

    return;
  }

  //## Инициализировать, настроить VBO и заполнить данными
  void Space::vbo_allocate_mem(void)
  {
    prog3d.attach_shaders(
      tr::Config::filepath(VERT_SHADER),
      tr::Config::filepath(FRAG_SHADER)
    );
    prog3d.use();

    // инициализация VAO
    glGenVertexArrays(1, &vao_3d);
    glBindVertexArray(vao_3d);

    // выделить память под VBO буферы
    // 1. Два базовых буфера формы частиц

    // Координаты вершин базового элемента в режиме STRIPE_TRIANGLE
    GLfloat arr_c3df[] = {
      -0.5f, 0.0f, +0.5f,
      +0.5f, 0.0f, +0.5f,
      -0.5f, 0.0f, -0.5f,
      +0.5f, 0.0f, -0.5f,
    };

    // Координаты текстуры для 4-x вершин
    GLfloat arr_txco[] = { 
      0.000f, 0.000f, 0.125f, 0.000f, 0.000f, 0.125f, 0.125f, 0.125f,
    };

    // Создать буфер координат базового элемента
    VBO v0 {};
    v0.Allocate(3 * sizeof(float) * 4, arr_c3df);
    v0.Attrib
      (prog3d.attrib_location_get("C3df"), 3, GL_FLOAT, GL_FALSE, 0, nullptr);

    // Создать буфер текстур базового элемента
    VBO v1 {};
    v1.Allocate(2 * sizeof(float) * 4, arr_txco);
    v1.Attrib
      (prog3d.attrib_location_get("TxCo"), 2, GL_FLOAT, GL_TRUE, 0, nullptr);

    // массив инстансов ----------------

    // Количество байт на группу атрибутов (3 координаты + 3 нормали):
    auto stride =  static_cast<GLsizei>(InstDataSize);

    // число элементов в кубе равно кубу длины стороны:
    auto n = pow(space_i0_length, 3);

    // В каждом блоке по инстансу, поэтому выделяем память под массив по числу блоков:
    VBO_Inst.Allocate(static_cast<GLsizeiptr>(n * stride));

    // Резервирование массива под хранение ссылок. Число элементов равно
    // числу Ригов в видимой зоне + 1 по каждой координате (под буфер кэша)
    auto reserved_size = static_cast<size_t>(pow(space_i0_length + 1, 3));
    visible_rigs = new Rig* [reserved_size];

    GLuint attrib = 0;
    GLvoid* pointer = nullptr;
    
    // 2.1 Центральная точка инстанса
    attrib = prog3d.attrib_location_get("Origin");
    pointer = nullptr;
    VBO_Inst.Attrib(attrib, 3, GL_FLOAT, GL_FALSE, stride, pointer);
    glVertexAttribDivisor(attrib, 1);

    // 2.2 Вектор нормали
    attrib = prog3d.attrib_location_get("Norm");
    pointer = reinterpret_cast<GLvoid*>(3 * sizeof(GLfloat));
    VBO_Inst.Attrib(attrib, 3, GL_FLOAT, GL_FALSE, stride, pointer);
    glVertexAttribDivisor(attrib, 1);

    glBindVertexArray(0);
    prog3d.unuse();

    return;
  }

  //## Построение границы области по оси X по ходу движения
  void Space::recalc_border_x(float direction, float VFx, float VFz)
  {
    // TODO: ВНИМАНИЕ! проверяется только 10 уровней Y
    //       надо добавить перестроение всего слоя на rigs_db.gage по Y
    Rig* r;
    float x = MoveFrom.x + space_f0_radius * direction;
    float Min = MoveFrom.z - space_f0_radius;
    float Max = MoveFrom.z + space_f0_radius;
    
    // Сбор индексов VBO по оси X с удаляемой линии границы области
    for(float z = Min; z <= Max; z += rigs_db0.gage)
      for(float y = -5.f; y <= 5.f; y += rigs_db0.gage)
      {
        r = rigs_db0.get(x, y, z);
        if(nullptr != r) cashe_vbo_ptr.push_back(r->vbo_offset);
      }

    x = VFx - space_f0_radius * direction;
    Min = VFz - space_f0_radius;
    Max = VFz + space_f0_radius;

    // Построение линии по оси X по направлению движения
    for(float z = Min; z <= Max; z += rigs_db0.gage)
      for(float y = -5.f; y <= 5.f; y += rigs_db0.gage)
        if(rigs_db0.exist(x, y, z))
          vbo_data_send(fround(x), fround(y), fround(z));
    
    MoveFrom.x = VFx;
    return;
  }

  //## Построение границы области по оси Z по ходу движения
  void Space::recalc_border_z(float direction, float VFx, float VFz)
  {
    // TODO: ВНИМАНИЕ! проверяется только 10 уровней Y
    //       надо добавить перестроение всего слоя на rigs_db.gage по Y
    Rig* r;
    float z = MoveFrom.z + space_f0_radius * direction;
    float Min = MoveFrom.x - space_f0_radius;
    float Max = MoveFrom.x + space_f0_radius;

    // Сбор индексов VBO по оси Z с удаляемой линии границы области
    for(float x = Min; x <= Max; x += rigs_db0.gage)
      for(float y = -5.f; y <= 5.f; y += rigs_db0.gage)
      {
        r = rigs_db0.get(x, y, z);
        if(nullptr != r) cashe_vbo_ptr.push_back(r->vbo_offset);
      }

    z = VFz - space_f0_radius * direction;
    Min = VFx - space_f0_radius;
    Max = VFx + space_f0_radius;

    // Построение линии по оси Z по направлению движения
    for(float x = Min; x <= Max; x += rigs_db0.gage)
      for(float y = -5.f; y <= 5.f; y += rigs_db0.gage)
        if(rigs_db0.exist(x, y, z))
          vbo_data_send(fround(x), fround(y), fround(z));
      
    MoveFrom.z = VFz;
    return;
  }
  
  //## Перестроение границ активной области при перемещении камеры
  void Space::recalc_borders(void)
  {
  // Функция вызвается при каждом изменении положения камеры.
  //
  // - cобираем в кэш список адресов VBO по которым (были) расположены блоки
  //    атрибутов (старых) элементов, вышедшие за границу отображения.
  //
  // - атрибуты новых элементов, которые следует добавить в сцену, заносим
  //   в VBO на места старых, перезаписывая их данные.
  //
  // - если новых элементов оказалось больше, то дописываем их в конец буфера.
  //
  // - если меньше, то пока весь кэш не очистится, на освободившееся место
  //   переносим данные из конца буфера и сдвигаем границу отображения, отсекая
  //   отрисовку лишних элементов
  //
  // - За границей конца буфера следит счетчик числа отображаемых элементов

    float
      VFx = fround(ViewFrom.x),
      VFz = fround(ViewFrom.z),
      dx = MoveFrom.x - VFx, // смещение камеры по оси x
      dz = MoveFrom.z - VFz; // смещение камеры по оси z
  
    float
      abs_dx = static_cast<float>(fabs(dx)),
      abs_dz = static_cast<float>(fabs(dz));

    if (abs_dx >= rigs_db0.gage) recalc_border_x(dx / abs_dx, VFx, VFz);
    if (abs_dz >= rigs_db0.gage) recalc_border_z(dz / abs_dz, VFx, VFz);
    
    // Очистка неиспользованных элементов
    if( 0 != cashe_vbo_ptr.size() ) reduce_keys();
    return;
  }

  //## Покадровое уменьшение счетчика для лишних инстансов
  void Space::reduce_keys(void)
  {
  // Функция вызывается при наличии данных в cashe_vbo_ptr
  //
  // Когда после перемещения камеры в графическом буфере остаются
  // неиспользованные при перестроении новых границ элементы, на их место
  // перемещаем атрибуты рабочих инстансов из конца буфера, и уменьшаем
  // счетчик отображаемых элементов, отсекая отображение лишних.
  //

    // Выбираем  крайний по счетчику границы индекс в VBO
    GLsizeiptr idSource = InstDataSize * (count - 1);

    // Если он оказался в кэше освободившихся, то просто сдвигаем границу,
    // уменьшая число элементов в счетчике и выходим,
    auto refSize = cashe_vbo_ptr.size();
    cashe_vbo_ptr.remove(idSource);
    if ( (refSize - cashe_vbo_ptr.size()) > 0 )
    {
       cutback();
       return;
     }
    
    // если нет, то выбираем из кэша первый свободный индекс (idTarget)
    GLsizeiptr idTarget = cashe_vbo_ptr.front();
    cashe_vbo_ptr.pop_front();

    // и переносим блок данных из конца VBO (idSource) на место idTarget.
    VBO_Inst.Reduce(idSource, idTarget, InstDataSize);
    cutback();
    
    auto id_ref_Source = static_cast<size_t>(idSource / InstDataSize);
    auto id_ref_Target = static_cast<size_t>(idTarget / InstDataSize);

    // Находим через массив visible_rigs активный риг и обновляем в его списке
    // адрес инстанса, который был перемещен в VBO_Inst.
    Rig* r = visible_rigs[id_ref_Source];
    if(nullptr == r) ERR("Failure in visible_rigs[id_ref_Source]");

    visible_rigs[id_ref_Source] = nullptr;
    visible_rigs[id_ref_Target] = r;

    r->vbo_offset = idTarget;

    return;
  }

  //## Декремент числа блоков данных в VBO и сдвиг границы
  void Space::cutback(void)
  {
    --count;
    // сдвиг границы актуальных данных в буфере
    VBO_Inst.Resize( InstDataSize * count );
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
    recalc_borders();
    return;
  }

  //## Расчет координат ближнего блока, на который направлен взгляд
  void Space::calc_selected_area(glm::vec3 & s_dir)
  {
  // временно отключено
     Selected = ViewFrom - s_dir;
     return;

  // ----------------------- отключено -----------------------------
    Selected = ViewTo;
    glm::vec3 check_step = { s_dir.x/8.f, s_dir.y/8.f, s_dir.z/8.f };
    for(int i = 0; i < 24; ++i)
    {
      if(rigs_db0.exist(Selected.x, Selected.y, Selected.z))
      {
        Selected.x = fround(Selected.x);
        Selected.y = fround(Selected.y);
        Selected.z = fround(Selected.z);
        break;
      }
      Selected += check_step;
    }

    return;
  }

  //## Функция, вызываемая из цикла окна для рендера сцены
  void Space::draw(const evInput & ev)
  {
    // Матрицу модели в расчетах не используем, так как
    // она единичная и на положение элементов влияние не оказывает

    calc_position(ev);

    prog3d.use();   // включить шейдерную программу
    prog3d.set_uniform("mvp", MatProjection * MatView);
    prog3d.set_uniform("Selected", Selected);
  
    glBindVertexArray(vao_3d);

    glActiveTexture(GL_TEXTURE0); // можно загрузить не меньше 48
    glBindTexture(GL_TEXTURE_2D, m_textureObj);

    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glEnable(GL_DEPTH_TEST);
    glDrawArraysInstanced(GL_TRIANGLE_STRIP, 0, 4, count);
    glBindVertexArray(0);
  
    prog3d.unuse(); // отключить шейдерную программу
    return;
  }

} // namespace tr
