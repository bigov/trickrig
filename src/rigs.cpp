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

  Vertex::Vertex(GLfloat *data)
  {
    position.x = data++;
    position.y = data++;
    position.z = data++;
    position.w = data++;
    color.r    = data++;
    color.g    = data++;
    color.b    = data++;
    color.a    = data++;
    normal.x   = data++;
    normal.y   = data++;
    normal.z   = data++;
    normal.w   = data++;
    fragment.u = data++;
    fragment.v = data++;
    return;
  }

  //## сдвиг прямоугольника
  void Quad::relocate(f3d & point)
  {
    *vertices[0].position.x += point.x;
    *vertices[1].position.x += point.x;
    *vertices[2].position.x += point.x;
    *vertices[3].position.x += point.x;

    *vertices[0].position.y += point.y;
    *vertices[1].position.y += point.y;
    *vertices[2].position.y += point.y;
    *vertices[3].position.y += point.y;

    *vertices[0].position.z += point.z;
    *vertices[1].position.z += point.z;
    *vertices[2].position.z += point.z;
    *vertices[3].position.z += point.z;

    return;
  }

  //## сдвиг индексов на опорную точку
  GLsizei *Quad::reindex(GLsizei stride)
  {
    for(size_t i = 0; i < num_indices; i++) idx[i] += stride;
    return idx;
  }

  Rig::Rig(f3d point, short t)
  {
    type = t;
    time = get_msec();
    area.relocate( point );
    return;
  }

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

  //## Вставка элемента в базу данных
  void Rigs::emplace(int x, int y, int z)
  {
    short block_type = 1; // тип блока по-умолчанию
    emplace(x, y, z, block_type);
    return;
  }

  //## Вставка в базу данных элемента c указанием типа
  void Rigs::emplace(int x, int y, int z, short type)
  {
    if (emplace_complete) ERR("Rigs emplaced is closed now.");
    f3d point = {x, y, z};
    db[point] = {point, type};
    return;
  }

} //namespace
