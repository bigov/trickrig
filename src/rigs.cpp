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

  //## конструктор по-умолчанию
  snip::snip(const tr::f3d & p)
  {
    relocate(p);
    if( .0f == p.x && .0f == p.z )
    {
      for(size_t i = 0; i < 4; i++)
        *vertices[i].fragment.v += .375f;
    }
  }

  //## сдвиг прямоугольника
  void snip::relocate(const tr::f3d & point)
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
  GLsizei *snip::reindex(GLsizei stride)
  {
    stride -= idx[0]; // для возврата индексов в исходное положение
    for(size_t i = 0; i < indices_per_snip; i++) idx[i] += stride;
    return idx;
  }

  //## добавление данных в буферы VBO
  void snip::vbo_append(tr::VBO &VBOdata, tr::VBO &VBOidx)
  {
  /* Добавляет данные в конец VBO буфера данных и VBO буфера индексов
   * и запоминает смещение адресов в VBO где данные были записаны
   */
    data_offset = VBOdata.SubDataAppend( tr::snip_data_size, data );
    idx_offset = VBOidx.SubDataAppend( tr::snip_index_size,
       reindex( data_offset / tr::snip_vertex_size ));
    return;
  }

  //## обновление данных в VBO буфере данных и VBO буфере индексов
  void snip::vbo_update(tr::VBO &VBOdata, tr::VBO &VBOidx, std::pair<GLsizeiptr, GLsizeiptr> &p)
  {
    data_offset = p.first;
    idx_offset = p.second;

    VBOdata.SubDataUpdate( tr::snip_data_size, data, p.first );
    VBOidx.SubDataUpdate( tr::snip_index_size,
      reindex( data_offset / tr::snip_vertex_size ), p.second );

    return;
  }

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
