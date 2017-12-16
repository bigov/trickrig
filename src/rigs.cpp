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
  //## конструктор по-умолчанию
  rig::rig(const tr::f3d &p)
  {
    time = get_msec();
    area.emplace_front(p);
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
        db.at(f3d {x, y, z});
        return f3d {x, y, z};
      } catch (...)
      { y -= gage; }
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
    try { return &db.at(P); }
    catch (...) { return nullptr; }
  }

  //## Вставка элемента в базу данных
  void rigs::emplace(int x, int y, int z)
  {
    f3d point = {x, y, z};
    db.emplace(std::make_pair(point, point));
    return;
  }

  //## Установка снипа по указаным координатам
  void rigs::set(const tr::f3d &P, std::forward_list<tr::snip> &A)
  {
    for(auto &S: A) S.point_set(P);

    tr::rig *R = get(P);
    if(nullptr == R)
    {
      db.emplace(std::make_pair(P, P));
      R = get(P);
    }

    R->area.clear();
    R->area.splice_after(R->area.before_begin(), A);
    return;
  }

} //namespace
