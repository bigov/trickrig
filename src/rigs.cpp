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

  //## Проверка наличия блока в заданных координатах
  bool Rigs::exist(float x, float y, float z)
  {
    if (nullptr == get(x, y, z)) return false;
    else return true;
  }

  //## Проверка отсутствия блока в заданных координатах
  bool Rigs::is_empty(float x, float y, float z)
  {
    return !exist(x, y, z);
  }

  //## поиск ближайшего нижнего блока по координатам точки
  //
  // При работе изменяет значение полученного аргумента.
  // Если объект найден, то аргумен содержит его координаты
  //
  f3d Rigs::search_down(const glm::vec3& v)
  { return search_down(v.x, v.y, v.z); }

  //## поиск ближайшего нижнего блока по координатам точки
  f3d Rigs::search_down(float x, float y, float z)
  {
    if(y < yMin) ERR("Y downflow"); if(y > yMax) ERR("Y overflow");

    x = fround(x); y = fround(y); z = fround(z);

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
  Rig* Rigs::get(float x, float y, float z)
  {
  //
  // Возвращает адрес памяти, в которой расположен элемент
  // с указанными в аргументе функции координатами.
  //
    if(y < yMin) ERR("Y downflow");
    if(y > yMax) ERR("Y overflow");

    try { return &db.at( {fround(x), fround(y), fround(z)}); }
    catch (...){ return nullptr; }
  }

  //## Вставка нового элемента в базу данных
  void Rigs::emplace(float x, float y, float z, short rig_type)
  {
    if (emplace_complete) ERR("Rigs emplaced is closed now.");
    // новые точки создаются только в местах без дробной части
    f3d point {fround(x), fround(y), fround(z)};
    db[point] = {rig_type};
    return;
  }

} //namespace
