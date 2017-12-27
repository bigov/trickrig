//============================================================================
//
// file: rigs.cpp
//
// Элементы формирования пространства
//
//============================================================================
#include "rigs.hpp"

namespace tr
{
  //## конструктор
  rig::rig(const tr::snip & Snip): born(get_msec())
  {
    Area.push_front(Snip);
    return;
  }

  //## конструктор
  rig::rig(const tr::f3d &p): born(get_msec())
  {
    add_snip(p);
    return;
  }

  //## конструктор
  rig::rig(int x, int y, int z): born(get_msec())
  {
    add_snip(tr::f3d{x,y,z});
    return;
  }

  //## конструктор копирующий
  rig::rig(const tr::rig & Other): born(get_msec())
  {
  /* При создании нового элемента метку времени
   * ставим по моменту создания объекта
   */
    copy_snips(Other);
    return;
  }

  //## Оператор присваивания это не конструктор
  tr::rig& rig::operator= (const tr::rig & Other)
  {
  /* При копировании существующего элемента в
   * созданый ранее объект метку времени копируем
   */
    if(this != &Other)
    {
      born = Other.born;
      Area.clear();
      copy_snips(Other);
    }
    return *this;
  }

  //## Копирование списка снипов из другого объекта
  void rig::copy_snips(const tr::rig & Other)
  {
    for(tr::snip Snip: Other.Area) Area.push_front(Snip);
    return;
  }

  //## Установка в указаной точке дефолтного снипа
  void rig::add_snip(const tr::f3d &p)
  {
    Area.emplace_front(p);
    return;
  }

  //## Установка масштаба и загрузка пространства из БД
  void rigs::init(float g)
  {
  // Метод ::init() ( вызывается из класса построения виртуального 3D
  // пространства ) обеспечивает формирование в оперативной памяти приложения
  // карту ( контейнер map() ) размещения ригов в трехмерных координатах для
  // необходимой области пространства.
  //
  // Данные для построения в графическом буфер VAO берутся из этой карты.
  // Таким образом, она играет роль своеобразного кэша к базе данных.

    db_gage *= g; // TODO проверка масштаба на допустимость

    //     Db.emplace(std::make_pair(f3d{x, y, z}, f3d{x, y, z}));

    /// Загрузка из Obj файла объекта в один риг:
    //tr::obj_load Obj = {"../assets/test_flat.obj"};
    //tr::f3d P = {1.f, 0.f, 1.f};
    //for(auto &S: Obj.Area) S.point_set(P); // Установить объект в точке P
    //get(P)->Area.swap(Obj.Area);           // Загрузить в rig(P)

    // загрузка из Obj файла по одной плоскости в каждый риг
    tr::obj_load Obj = {"../assets/surf16x16.obj"};

    tr::f3d Base = {0.0f, 0.0f, 0.0f};
    tr::f3d Pt   = {0.0f, 0.0f, 0.0f};

    for(int z = -4; z < 4; z ++)
    {
      Base.z = static_cast<float>(z * 16 + 8);

    for(int x = -4; x < 4; x ++)
    {
      Base.x = static_cast<float>(x * 16 + 8);

      for(tr::snip S: Obj.Area)
      {
        size_t n = 0;
        // В снипе 4 вершины. Найдем индекс опорной (по минимальному значению)
        for(size_t i = 1; i < 4; i++)
          if((
               S.data[n * ROW_STRIDE + COORD_X]
             + S.data[n * ROW_STRIDE + COORD_Y]
             + S.data[n * ROW_STRIDE + COORD_Z]
                ) > (
               S.data[i * ROW_STRIDE + COORD_X]
             + S.data[i * ROW_STRIDE + COORD_Y]
             + S.data[i * ROW_STRIDE + COORD_Z]
             )) n = i;

        // Координаты найденой вершины используем для создания рига
        Pt.x = floor(S.data[n * ROW_STRIDE + COORD_X]) + Base.x;
        Pt.y = floor(S.data[n * ROW_STRIDE + COORD_Y]) + Base.y;
        Pt.z = floor(S.data[n * ROW_STRIDE + COORD_Z]) + Base.z;

        S.point_set(Pt);
        Db[Pt] = tr::rig { S };
      }
    } //for x
    } //for z

    // Выделить текстурой центр координат
    get(0,0,0 )->Area.front().texture_set(0.125, 0.125 * 4.0);

    return;
  }

  //## Проверка наличия блока в заданных координатах
  bool rigs::exist(float x, float y, float z)
  {
    if (nullptr == get(x, y, z)) return false;
    else return true;
  }

  //## Проверка отсутствия блока в заданных координатах
  bool rigs::is_empty(float x, float y, float z)
  {
    return !exist(x, y, z);
  }

  //## поиск ближайшего нижнего блока по координатам точки
  //
  // При работе изменяет значение полученного аргумента.
  // Если объект найден, то аргумен содержит его координаты
  //
  f3d rigs::search_down(const glm::vec3& v)
  { return search_down(v.x, v.y, v.z); }

  //## поиск ближайшего нижнего блока по координатам точки
  f3d rigs::search_down(float x, float y, float z)
  {
    if(y < yMin) ERR("Y downflow"); if(y > yMax) ERR("Y overflow");

    x = floor(x); y = floor(y); z = floor(z);

    while(y > yMin)
    {
      try
      { 
        Db.at(f3d {x, y, z});
        return f3d {x, y, z};
      } catch (...)
      { y -= db_gage; }
    }
    ERR("Rigs::search_down() failure. You need try/catch in this case.");
  }

  //## Поиск элемента с указанными координатами
  rig* rigs::get(float x, float y, float z)
  {
    return get(tr::f3d{x, y, z});
  }

  //## Поиск элемента с указанными координатами
  rig* rigs::get(const tr::f3d &P)
  {
    if(P.y < yMin) ERR("rigs::get -Y is overflow");
    if(P.y > yMax) ERR("rigs::get +Y is overflow");
    // Вначале поищем как указано.
    try { return &Db.at(P); }
    catch (...) { return nullptr; }
  }

} //namespace
