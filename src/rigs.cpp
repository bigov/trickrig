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

  // Замена указанного значения в списке
  void Rig::idx_update(GLsizeiptr idSource, GLsizeiptr idTarget)
  {
    for(auto& id: idx)
      if(id == idSource)
      {
        id = idTarget;
        return;
      }
    return;
  }

  //## Создание маски отображаемых сторон по наличию соседних элементов
  //
  unsigned char Rigs::sides_map(const f3d& addr)
  {
    // координаты ближайшей центральной точки, с ней и работаем
    f3d pt {fround(addr.x), fround(addr.y), fround(addr.z)};

    Rig* r = get(pt);
    if(nullptr == r) ERR("ERR: call sides_map for empty space.");
    if(r->neighbors & TR_T00)
    { // просканировать соседей, если это еще не выполнено
      r->neighbors = 0;
      if(is_empty(pt.x + gage, pt.y, pt.z)) r->neighbors |= TR_TpX;
      if(is_empty(pt.x - gage, pt.y, pt.z)) r->neighbors |= TR_TnX;
      if(is_empty(pt.x, pt.y + gage, pt.z)) r->neighbors |= TR_TpY;
      if(is_empty(pt.x, pt.y - gage, pt.z)) r->neighbors |= TR_TnY;
      if(is_empty(pt.x, pt.y, pt.z + gage)) r->neighbors |= TR_TpZ;
      if(is_empty(pt.x, pt.y, pt.z - gage)) r->neighbors |= TR_TnZ;
    }
    return r->neighbors;
  }

  //## Сохранение индекса адреса размещения атрибутов из VBO_Inst
  //
  void Rigs::post_key(const f3d& c, GLsizeiptr i)
  {
    Rig *r = get(c);
    if(nullptr == r) ERR("ERR: call post_key for empty space.");
    r->idx.push_back(i);
    return;
  }

  //## Наличия теней на кромках
  //
  // кодируется в ТРОИЧНОЙ системе счисления:
  //
  //  2 - если должна быть тень (угол 90 градусов)
  //  1 - ровное освещение (плоский стык сосединих блоков)
  //  0 - осветление кромки - нет стыка
  //
  GLint Rigs::edges_map(const f3d& addr, const unsigned char side)
  {
    GLint result = 0;

    // координаты ближайшей центральной точки, с ней и работаем
    f3d c {fround(addr.x), fround(addr.y), fround(addr.z)};

    switch (side)
    {
      case TR_TpX:
        if (!is_empty(c.x+gage, c.y+gage, c.z)) result += 2;
        else if (!is_empty(c.x, c.y+gage, c.z)) result += 1;
        if (!is_empty(c.x+gage, c.y-gage, c.z)) result += 6;
        else if (!is_empty(c.x, c.y-gage, c.z)) result += 3;
        if (!is_empty(c.x+gage, c.y, c.z+gage)) result += 18;
        else if (!is_empty(c.x, c.y, c.z+gage)) result += 9;
        if (!is_empty(c.x+gage, c.y, c.z-gage)) result += 54;
        else if (!is_empty(c.x, c.y, c.z-gage)) result += 27;
        break;
      case TR_TnX:
        if (!is_empty(c.x-gage, c.y-gage, c.z)) result += 2;
        else if (!is_empty(c.x, c.y-gage, c.z)) result += 1;
        if (!is_empty(c.x-gage, c.y+gage, c.z)) result += 6;
        else if (!is_empty(c.x, c.y+gage, c.z)) result += 3;
        if (!is_empty(c.x-gage, c.y, c.z+gage)) result += 18;
        else if (!is_empty(c.x, c.y, c.z+gage)) result += 9;
        if (!is_empty(c.x-gage, c.y, c.z-gage)) result += 54;
        else if (!is_empty(c.x, c.y, c.z-gage)) result += 27;
        break;
      case TR_TpY:
        if (!is_empty(c.x-gage, c.y+gage, c.z)) result += 2;
        //else result += 1;
        else if (!is_empty(c.x-gage, c.y, c.z)) result += 1;
        if (!is_empty(c.x+gage, c.y+gage, c.z)) result += 6;
        //else result += 3;
        else if (!is_empty(c.x+gage, c.y, c.z)) result += 3;
        if (!is_empty(c.x, c.y+gage, c.z+gage)) result += 18;
        //else result += 9;
        else if (!is_empty(c.x, c.y, c.z+gage)) result += 9;
        if (!is_empty(c.x, c.y+gage, c.z-gage)) result += 54;
        //else result += 27;
        else if (!is_empty(c.x, c.y, c.z-gage)) result += 27;
        break;
      case TR_TnY: //TODO
        break;
      case TR_TpZ:
        if (!is_empty(c.x-gage, c.y, c.z+gage)) result += 2;
        else if (!is_empty(c.x-gage, c.y, c.z)) result += 1;
        if (!is_empty(c.x+gage, c.y, c.z+gage)) result += 6;
        else if (!is_empty(c.x+gage, c.y, c.z)) result += 3;
        if (!is_empty(c.x, c.y-gage, c.z+gage)) result += 18;
        else if (!is_empty(c.x, c.y-gage, c.z)) result += 9;
        if (!is_empty(c.x, c.y+gage, c.z+gage)) result += 54;
        else if (!is_empty(c.x, c.y+gage, c.z)) result += 27;
        break;
      case TR_TnZ:
        if (!is_empty(c.x-gage, c.y, c.z-gage)) result += 2;
        else if (!is_empty(c.x-gage, c.y, c.z)) result += 1;
        if (!is_empty(c.x+gage, c.y, c.z-gage)) result += 6;
        else if (!is_empty(c.x+gage, c.y, c.z)) result += 3;
        if (!is_empty(c.x, c.y+gage, c.z-gage)) result += 18;
        else if (!is_empty(c.x, c.y+gage, c.z)) result += 9;
        if (!is_empty(c.x, c.y-gage, c.z-gage)) result += 54;
        else if (!is_empty(c.x, c.y-gage, c.z)) result += 27;
        break;
    }
    return result;
  }

  //## Проверка наличия блока в заданных координатах
  //
  bool Rigs::exist(float x, float y, float z)
  {
    if (nullptr == get(x, y, z)) return false;
    else return true;
  }

  //## Проверка отсутствия блока в заданных координатах
  //
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
  //
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
  //
  // Возвращает адрес памяти, в которой расположен элемент
  // с указанными в аргументе функции координатами.
  //
  Rig* Rigs::get(const f3d& c) { return get(c.x, c.y, c.z); }
  Rig* Rigs::get(float x, float y, float z)
  {
    if(y < yMin) ERR("Y downflow");
    if(y > yMax) ERR("Y overflow");

    try { return &db.at( {fround(x), fround(y), fround(z)}); }
    catch (...){ return nullptr; }
  }

  //## Вставка нового элемента в базу данных
  //
  void Rigs::emplace(float x, float y, float z, short t)
  {
    if (emplace_complete) ERR("Rigs emplaced is closed now.");
    // новые точки создаются только в местах без дробной части
    f3d point {fround(x), fround(y), fround(z)};
    db[point] = {t};
    return;
  }

} //namespace
