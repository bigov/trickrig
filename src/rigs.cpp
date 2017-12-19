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
    db_gage *= g; // TODO проверка масштаба на допустимость

    int s = 10;

    int x0 = static_cast<int>(floor(tr::Cfg.ViewFrom.x));
    int y = 0;
    int z0 = static_cast<int>(floor(tr::Cfg.ViewFrom.z)) - s;

    for(int x = x0 - s; x < s; x += 1)
      for(int z = z0 - s; z < s; z += 1)
        Db.emplace(std::make_pair(f3d{x, y, z}, f3d{x, y, z}));

    // Выделить текстурами центр и оси координат
    get(0,0,0 )->Area.front().texture_set(0.125, 0.125*7);
    get(1,0,0 )->Area.front().texture_set(0.125, 0.0);
    get(-1,0,0)->Area.front().texture_set(0.125, 0.125);
    get(0,0,1 )->Area.front().texture_set(0.125, 0.125*4);
    get(0,0,-1)->Area.front().texture_set(0.125, 0.125*5);

    // Загрузить объект из внешнего файла
    tr::loader_obj Obj = {"../assets/test_flat.obj"};

    tr::f3d P = {1.f, 0.f, 1.f};
    for(auto &S: Obj.Area) S.point_set(P); // Установить объект в точке P
    get(P)->Area.swap(Obj.Area);           // Загрузить в rig(P)

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
