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
  float yMin = -100.f;
  float yMax = 100.f;

  //## конструктор по-умолчанию
  rig::rig(const tr::f3d &p)
  {
    time = get_msec();
    area.emplace_front(p);
    return;
  }

  //## Загрузчик в риг новых данных
  /*
  void rig::reload(const std::vector<std::pair<
    std::array<float, 3>,std::array<float, 3>>> & Vertices)
  {
    area.clear();
    for(auto &v: Vertices)
    {
      tr::snip Snip {};
      Snip.vertices[0].position.x = v.first[0];
      Snip.vertices[0].position.y = v.first[1];
      Snip.vertices[0].position.z = v.first[2];
    }
    return;
  }*/

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
  //
  // Возвращает адрес памяти, в которой расположен элемент
  // с указанными в аргументе функции координатами.
  //
    if(y < yMin) ERR("rigs::get -Y is overflow");
    if(y > yMax) ERR("rigs::get +Y is overflow");
    // Вначале поищем как указано.
    try { return &db.at({x, y, z}); }
    catch (...) { return nullptr; }
  }

  //## Вставка элемента в базу данных
  void rigs::emplace(int x, int y, int z)
  {
    f3d point = {x, y, z};
    db.emplace(std::make_pair(point, point));
    return;
  }

} //namespace
