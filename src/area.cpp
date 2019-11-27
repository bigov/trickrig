/**
 *
 * file: area.cpp
 *
 * Класс управления элементами области 3D пространства
 *
 */

#include "area.hpp"

namespace tr
{

///
/// \brief area::area
/// \param length - длина стороны вокселя
/// \param count - число вокселей от камеры (или внутренней границы)
/// до внешней границы области
///
area::area(int len, int elements, std::shared_ptr<voxesdb> V): Voxes(V)
{
  assert(Voxes);
  vox_side_len = len;
  lod_dist_far = elements * len;
}


///
/// \brief area::load
/// \param ViewFrom
///
void area::load(const glm::vec3& ViewFrom)
{
  // Origin вокселя, в котором расположена камера
  Location = { static_cast<int>(floorf(ViewFrom.x / vox_side_len)) * vox_side_len,
               static_cast<int>(floorf(ViewFrom.y / vox_side_len)) * vox_side_len,
               static_cast<int>(floorf(ViewFrom.z / vox_side_len)) * vox_side_len };
  MoveFrom = Location;

  i3d P0 { Location.x - lod_dist_far,
           Location.y - lod_dist_far,
           Location.z - lod_dist_far };
  i3d P1 { Location.x + lod_dist_far,
           Location.y + lod_dist_far,
           Location.z + lod_dist_far };

  i3d vP {};
  for(vP.x = P0.x; vP.x<= P1.x; vP.x += vox_side_len)
  for(vP.y = P0.y; vP.y<= P1.y; vP.y += vox_side_len)
  for(vP.z = P0.z; vP.z<= P1.z; vP.z += vox_side_len) Voxes->load(vP);
}


///
/// \brief space::recalc_borders
/// \details Перестроение границ активной области при перемещении камеры
///
void area::recalc_borders(const glm::vec3& ViewFrom)
{
  // Origin вокселя, в котором расположена камера
  Location = { static_cast<int>(floorf(ViewFrom.x / vox_side_len)) * vox_side_len,
               static_cast<int>(floorf(ViewFrom.y / vox_side_len)) * vox_side_len,
               static_cast<int>(floorf(ViewFrom.z / vox_side_len)) * vox_side_len };

  if(Location.x != MoveFrom.x) redraw_borders_x();
  if(Location.z != MoveFrom.z) redraw_borders_z();

  queue_release();
}


///
/// \brief vox_buffer::append
/// \param i
///
/// \details Добавление вокса к указанной стороне
///
void area::append(u_int id)
{
  if(id > (Voxes->render_indices/indices_per_side) * bytes_per_side) return;
  GLsizeiptr offset = (id/vertices_per_side) * bytes_per_side;

  Voxes->append(offset);
}


///
/// \brief vox_buffer::remove
/// \param i - порядковый номер группы данных из буфера
///
/// \details Удаление вокса
///
void area::remove(u_int i)
{
  if(i > (Voxes->render_indices/indices_per_side) * bytes_per_side) return;
  GLsizeiptr offset = (i/vertices_per_side) * bytes_per_side; // по номеру группы - адрес смещения в VBO
  Voxes->remove(offset);
}


///
/// \brief area::queue_release
///
/// \details Операции с базой данных по скорости гораздо медленнее, чем рендер объектов из
/// оперативной памяти, поэтому обмен информацией с базой данных и выгрузка больших объемов
/// данных из памяти (при перестроении границ LOD) производится покадрово через очередь
/// фиксированными по продолжительности порциями.
void area::queue_release(void)
{
  auto st = std::chrono::system_clock::now();
  while ((std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now() - st).count() < 5))
  {
    // В работе постоянно две очереди: по ходу движения камеры очередь загрузки,
    // а с противоположной стороны - очередь выгрузки элементов.

    // Загрузка элементов пространства в графический буфер
    if(!QueueLoad.empty())
    {
      Voxes->load(QueueLoad.front()); // Загрузка данных имеет приоритет. Пока вся очередь
      QueueLoad.pop();                // загрузки не очистится, очередь выгрузки ждет.
      continue;
    }

    // Удаление из памяти данных тех элементов, которые в результате перемещения
    // камеры вышли за границу отображения текущего LOD
    if(!QueueWipe.empty())
    {
      Voxes->unload(QueueWipe.front());
      QueueWipe.pop();
    } else { return; }
  }
}


///
/// \brief space::redraw_borders_x
/// \details Построение границы области по оси X по ходу движения
///
void area::redraw_borders_x(void)
{
  int x_show, x_hide;
  if(Location.x > MoveFrom.x)
  {        // Если направление движение по оси Х
    x_show = Location.x + lod_dist_far; // X-линия вставки вокселей на границе LOD
    x_hide = MoveFrom.x - lod_dist_far; // X-линия удаления вокселей на границе
  } else { // Если направление движение против оси Х
    x_show = Location.x - lod_dist_far; // X-линия вставки вокселей на границе LOD
    x_hide = MoveFrom.x + lod_dist_far; // X-линия удаления вокселей на границе
  }

  for(int y = -lod_dist_far; y <= lod_dist_far; y += vox_side_len)
  {
    // Скрыть элементы с задней границы области
    for(int z = MoveFrom.z - lod_dist_far, lod = MoveFrom.z + lod_dist_far;
        z <= lod; z += vox_side_len) QueueWipe.push({x_hide, y, z});

    // Добавить линию элементов по направлению движения
    for(int z = Location.z - lod_dist_far, lod = Location.z + lod_dist_far;
        z <= lod; z += vox_side_len) QueueLoad.push({x_show, y, z});
  }

  MoveFrom.x = Location.x;
}


///
/// \brief space::redraw_borders_z
/// \details Построение границы области по оси Z по ходу движения
///
void area::redraw_borders_z(void)
{
  int yMin = -lod_dist_far;              // Y-граница LOD
  int yMax =  lod_dist_far;

  int z_show, z_hide;
  if(Location.z > MoveFrom.z)
  {        // Если направление движение по оси Z
    z_show = Location.z + lod_dist_far; // Z-линия вставки вокселей на границе LOD
    z_hide = MoveFrom.z - lod_dist_far; // Z-линия удаления вокселей на границе
  } else { // Если направление движение против оси Z
    z_show = Location.z - lod_dist_far; // Z-линия вставки вокселей на границе LOD
    z_hide = MoveFrom.z + lod_dist_far; // Z-линия удаления вокселей на границе
  }

  int xMin, xMax;

  // Скрыть элементы с задней границы области
  xMin = MoveFrom.x - lod_dist_far;
  xMax = MoveFrom.x + lod_dist_far;
  for(int y = yMin; y <= yMax; y += vox_side_len)
    for(int x = xMin; x <= xMax; x += vox_side_len) QueueWipe.push({x, y, z_hide});

  // Добавить линию элементов по направлению движения
  xMin = Location.x - lod_dist_far;
  xMax = Location.x + lod_dist_far;
  for(int y = yMin; y <= yMax; y += vox_side_len)
    for(int x = xMin; x <= xMax; x += vox_side_len) QueueLoad.push({x, y, z_show});

  MoveFrom.z = Location.z;
}


} //namespace
