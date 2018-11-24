//============================================================================
//
// file: rigs.cpp
//
// Элементы формирования пространства
//
//============================================================================
#include "rdb.hpp"

namespace tr
{
  //## Дублирующий конструктор
  rig::rig(const tr::rig & Other)
  {
    born = Other.born;
    for(int i = 0; i < SHIFT_DIGITS; i++) shift[i] = Other.shift[i];
    Trick.clear();
    for(tr::snip Snip: Other.Trick) Trick.push_front(Snip);
    return;
  }

  //## Оператор присваивания (это не конструктор)
  tr::rig& rig::operator= (const tr::rig & Other)
  {
    if(this != &Other)
    {
      born = get_msec();
      for(int i = 0; i < SHIFT_DIGITS; i++) shift[i] = Other.shift[i];
      Trick.clear();
      for(tr::snip Snip: Other.Trick) Trick.push_front(Snip);
    }
    return *this;
  }

  /// /// /// /// /// /// /// /// /// /// /// /// /// /// /// /// /// /// /// ///

  //## КОНСТРУКТОР
  rdb::rdb(void)
  {
    Prog3d.attach_shaders(
      tr::cfg::get(SHADER_VERT_SCENE),
      tr::cfg::get(SHADER_FRAG_SCENE)
    );
    Prog3d.use();  // слинковать шейдерную рограмму

    // инициализация VAO
    glGenVertexArrays(1, &space_vao);
    glBindVertexArray(space_vao);

    // Число элементов в кубе с длиной стороны = "space_i0_length" элементов:
    unsigned n = pow((tr::lod_0 + tr::lod_0 + 1), 3);

    // число байт для заполнения такого объема прямоугольниками:
    VBOdata.allocate(n * tr::bytes_per_snip);

    // настройка положения атрибутов
    VBOdata.attrib(Prog3d.attrib_location_get("position"),
      4, GL_FLOAT, GL_FALSE, tr::bytes_per_vertex, (void*)(0 * sizeof(GLfloat)));

    VBOdata.attrib(Prog3d.attrib_location_get("color"),
      4, GL_FLOAT, GL_TRUE, tr::bytes_per_vertex, (void*)(4 * sizeof(GLfloat)));

    VBOdata.attrib(Prog3d.attrib_location_get("normal"),
      4, GL_FLOAT, GL_TRUE, tr::bytes_per_vertex, (void*)(8 * sizeof(GLfloat)));

    VBOdata.attrib(Prog3d.attrib_location_get("fragment"),
      2, GL_FLOAT, GL_TRUE, tr::bytes_per_vertex, (void*)(12 * sizeof(GLfloat)));

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
    tr::vbo VBOindex = { GL_ELEMENT_ARRAY_BUFFER };                // Создать индексный буфер
    VBOindex.allocate(static_cast<GLsizei>(idx_size), idx_data);   // и заполнить данными.
    delete[] idx_data;                                             // Удалить исходный массив.

    glBindVertexArray(0);
    Prog3d.unuse();

    return;

  }

  //##  Рендер кадра
  void rdb::draw(const glm::mat4 & MVP)
  {
    if (!CachedOffset.empty()) clear_cashed_snips();

    Prog3d.use();   // включить шейдерную программу
    Prog3d.set_uniform("mvp", MVP);
    Prog3d.set_uniform("light_direction", glm::vec4(0.2f, 0.9f, 0.5f, 0.0));
    Prog3d.set_uniform("light_bright", glm::vec4(0.5f, 0.5f, 0.5f, 0.0));

    glBindVertexArray(space_vao);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glEnable(GL_DEPTH_TEST);
    glDrawElements(GL_TRIANGLES, render_points, GL_UNSIGNED_INT, nullptr);
    glBindVertexArray(0);
    Prog3d.unuse(); // отключить шейдерную программу

    return;
  }

  /// Размещение в графическом буфере данных, описывающих элементы
  /// виртуального 3D пространства, расположенные в точке (x, y, z)
  ///
  void rdb::put_in_vbo(int x, int y, int z)
  {
    tr::rig *Rig = get(x, y, z);
    if(nullptr == Rig) return;
    if(Rig->in_vbo) return;

    for(tr::snip &Snip: Rig->Trick)
    {
      tr::f3d Point = {
        static_cast<float>(x) + Rig->shift[SHIFT_X],
        static_cast<float>(y) + Rig->shift[SHIFT_Y],
        static_cast<float>(z) + Rig->shift[SHIFT_Z]  // TODO: еще есть поворот и zoom
      };

      bool data_is_recieved = false;
      while (!data_is_recieved)
      {
        if(CachedOffset.empty()) // Если кэш пустой, то добавляем данные в конец VBO
        {
          Snip.vbo_append(Point, VBOdata);
          render_points += tr::indices_per_snip;  // увеличить число точек рендера
          VisibleSnips[Snip.data_offset] = &Snip; // добавить ссылку
          data_is_recieved = true;
        }
        else // если в кэше есть адреса свободных мест, то используем
        {    // их с контролем, успешно ли был перемещен блок данных
          data_is_recieved = Snip.vbo_update(Point, VBOdata, CachedOffset.front());
          CachedOffset.pop_front();                           // укоротить кэш
          if(data_is_recieved) VisibleSnips[Snip.data_offset] = &Snip; // добавить ссылку
        }
      }
    }
    Rig->in_vbo = true;
    return;
  }

  //## убрать риг из рендера
  void rdb::remove_from_vbo(int x, int y, int z)
  {
  /// Индексы размещенных в VBO данных, которые при перемещении камеры вышли
  /// за границу отображения, запоминаются в кэше, чтобы на их место
  /// записать данные вершин, которые вошли в поле зрения с другой стороны.
  ///
    tr::rig * Rig = get(x, y, z);
    if(nullptr == Rig) return;
    if(!Rig->in_vbo) return;

    for(auto & Snip: Rig->Trick)
    {
      VisibleSnips.erase(Snip.data_offset);
      CachedOffset.push_front(Snip.data_offset);
    }
    Rig->in_vbo = false;
    return;
  }

  //## Удаление элементов по адресам с кэше и сжатие данных в VBO
  void rdb::clear_cashed_snips(void)
  {
  /// Если в кэше есть адрес блока из середины VBO, то в него переносим данные
  /// из конца VBO и сжимаем буфер на длину одного блока. Если адрес из кэша
  /// указывает на крайний блок в конце VBO, то сжимаем буфер сдвигая границу
  /// на длину одного блока.
  ///
  /// Не забываем уменьшить число элементов в рендере.

    // Выбрать самый крайний элемент VBO на границе блока данных
    GLsizeiptr data_src = VBOdata.get_hem();

    if(0 == render_points)
    {
      CachedOffset.clear();   // очистить все, сообщить о проблеме
      VisibleSnips.clear();   // и закончить обработку кэша
      return;
    }

    #ifndef NDEBUG
    if (data_src == 0 ) {      // Если (вдруг!) данных нет, то
      CachedOffset.clear();   // очистить все, сообщить о проблеме
      VisibleSnips.clear();   // и закончить обработку кэша
      render_points = 0;
      info("WARNING: space::clear_cashed_snips got empty data_src\n");
      return;
    }

    /// Граница буфера (VBOdata.get_hem()), если она не равна нулю,
    /// не может быть меньше размера блока данных (bytes_per_snip)
    if(data_src < tr::bytes_per_snip)
      ERR ("BUG!!! space::jam_vbo got error address in VBO");
    #endif

    data_src -= tr::bytes_per_snip; // адрес последнего блока

    /// Если крайний блок не в списке VisibleSnips, то он и не в рендере.
    /// Поэтому просто отбросим его, сдвинув границу буфера VBO. Кэш не
    /// изменяем, так как в контейнере "forward_list" удаление элементов
    /// из середины списка - затратная операция.
    ///
    /// Внимание! Так как после этого где-то в кэше остается невалидный
    /// (за рабочей границей VBO) адрес блока, то при использовании
    /// адресов из кэша надо делать проверку - не "протухли" ли они.
    ///
    if(VisibleSnips.find(data_src) == VisibleSnips.end())
    {
      VBOdata.shrink(tr::bytes_per_snip);   // укоротить VBO данных
      render_points -= tr::indices_per_snip; // уменьшить число точек рендера
      return;                                // и прервать обработку кэша
    }

    GLsizeiptr data_dst = data_src;
    // Извлечь из кэша адрес
    while(data_dst >= data_src)
    {
      if(CachedOffset.empty()) return;
      data_dst = CachedOffset.front();
      CachedOffset.pop_front();

      // Идеальный вариант, когда освободившийся блок оказался крайним в VBO
      if(data_dst == data_src)
      {
        VBOdata.shrink(tr::bytes_per_snip);    // укоротить VBO данных
        render_points -= tr::indices_per_snip; // уменьшить число точек рендера
        return;                                // закончить шаг обработки
      }
    }

    // Самый частый и самый сложный вариант
    try { // Если есть отображаемый data_src и меньший data_dst из кэша, то
      tr::snip *Snip = VisibleSnips.at(data_src);  // найти перемещаемый снип,
      VisibleSnips.erase(data_src);                // удалить его из карты
      Snip->vbo_jam(VBOdata, data_dst);            // переместить данные в VBO
      VisibleSnips[data_dst] = Snip;               // внести ссылку в карту
      render_points -= tr::indices_per_snip;       // Так как данные из хвоста
    } catch(std::exception & e) {
      ERR(e.what());
    } catch(...) {
      ERR("rigs::clear_cashed_snips got error VisibleSnips[data_src]");
    }
    return;
  }

  //## Загрузка из БД данных указанного рига
  tr::rig rdb::load_rig(int x, int y, int z)
  {
    //char buf_query[255];
    std::vector<unsigned char> BufVector = {};

    tr::sqlw DB = {};
    DB.open(tr::cfg::get(DB_TPL_FNAME));
    DB.select_rig(x, y, z);

    auto Row = DB.Rows.front();
    tr::rig Rig;
    Rig.born = std::any_cast<int>(Row[0]);

    BufVector.clear();
    BufVector = std::any_cast<std::vector<unsigned char>>(Row[2]);
    memcpy(Rig.shift, BufVector.data(), SHIFT_DIGITS * sizeof(float));
    DB.select_snip( std::any_cast<int>(Row[1]) );

    for(auto Row: DB.Rows)
    {
      tr::snip Snip = {};
      BufVector.clear();
      BufVector = std::any_cast<std::vector<unsigned char>>(Row[0]);
      memcpy(Snip.data, BufVector.data(), tr::bytes_per_snip);
      Rig.Trick.push_front(Snip);
    }
    DB.close();
    return Rig;
  }

  ///
  /// \brief Инициализация карты пространства
  ///
  /// \details  Формирование в оперативной памяти карты (std::map) ригов в
  /// трехмерных координатах для выбраной области пространства. Из этой карты
  /// берутся данные снипов, размещаемых в VBO для рендера сцены.
  ///
  void rdb::init(int g, i3d)
  {
    lod = g; // TODO проверка масштаба на допустимость
    //_load_16x16_obj();

    int y = 0;
    int tpl_side = 16; // длина стороны шаблона

    // загружаем шаблон поверхности 16x16
    for(int x = 0; x < tpl_side; x++) for(int z = 0; z < tpl_side; z++)
        TplRigs[tr::i3d{x, y, z}] = load_rig(x, y, z);

    // этот шаблон дублируем 8х8 раз на xz плоскости
    int row_x = 0, row_z = 0;
    for (int zn = -4; zn < 4; zn++)
    {
      row_z = zn * tpl_side;
      for (int xn = -4; xn < 4; xn++)
      {
        row_x = xn * tpl_side;
        for(int x = 0; x < tpl_side; x++) for(int z = 0; z < tpl_side; z++)
        {
          RigsDb[tr::i3d{row_x + x, y, row_z + z}] = TplRigs[tr::i3d{x, y, z}];
        }
      }
    }
    return;
  }

  //## сохранение блока ригов в базу данных
  bool rdb::save(const tr::i3d & From, const tr::i3d & To)
  {
  /// Вначале записываем в таблицу снипов данные области рига. При этом
  /// индекс области, который будет внесен в таблицу ригов, назначаем
  /// по номеру записи первого снипа группы области.
  ///
  /// После этого обновляем/вставляем запись в таблицу ригов с указанием
  /// индекса созданой группы
  ///

    tr::sqlw DB = {};
    DB.open(tr::cfg::get(DB_TPL_FNAME));

    int id_area = 0;
    //char query_buf[255];

    for(int x = From.x; x < To.x; x++)
      for(int y = From.y; y < To.y; y++)
        for(int z = From.z; z < To.z; z++)
        {
          tr::rig *R = get(x, y, z);
          if(nullptr != R)
          {
            id_area = 0;
            for(auto & Snip: R->Trick)
            {
              // Запись снипа
              DB.insert_snip(id_area, Snip.data);

              if(0 == id_area)
              {
                id_area = DB.Result.rowid;
                DB.update_snip( id_area, id_area ); //TODO: !!!THE BUG???
                // Обновить номер группы в записи первого снипа
              }
            }
            // Запись рига
            DB.insert_rig( x, y, z, R->born, id_area, R->shift, SHIFT_DIGITS);
            //DB.request_put(query_buf, R->shift, SHIFT_DIGITS);
          }
        }
    DB.close();
    return true;
  }

  //## Поиск по координатам ближайшего блока снизу
  tr::i3d rdb::search_down(const glm::vec3& V)
  {
    return search_down(V.x, V.y, V.z);
  }

  //## Поиск по координатам ближайшего блока снизу
  tr::i3d rdb::search_down(float x, float y, float z)
  {
    return search_down(
          static_cast<double>(x),
          static_cast<double>(y),
          static_cast<double>(z)
    );
  }

  //## Поиск по координатам ближайшего блока снизу
  tr::i3d rdb::search_down(double x, double y, double z)
  {
    return search_down(
      static_cast<int>(floor(x)),
      static_cast<int>(floor(y)),
      static_cast<int>(floor(z))
    );
  }

  //## Поиск по координатам ближайшего блока снизу
  tr::i3d rdb::search_down(int x, int y, int z)
  {
    if(y < yMin) ERR("Y downflow"); if(y > yMax) ERR("Y overflow");
    while(y > yMin)
    {
      try
      { 
        RigsDb.at(tr::i3d {x, y, z});
        return tr::i3d {x, y, z};
      } catch (...)
      { y -= lod; }
    }
    ERR("Rigs::search_down() failure. You need try/catch in this case.");
  }

  //## Поиск элемента с указанными координатами
  tr::rig* rdb::get(int x, int y, int z)
  {
    return get(tr::i3d{x, y, z});
  }

  //## Поиск элемента с указанными координатами
  tr::rig* rdb::get(const i3d &P)
  {
    if(P.y < yMin) ERR("rigs::get -Y is overflow");
    if(P.y > yMax) ERR("rigs::get +Y is overflow");

    try { return &RigsDb.at(P); }
    catch (...) { return nullptr; }
  }

} //namespace
