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
area::area(int len, int elements, std::shared_ptr<voxesdb> V, const glm::vec3& Pt): Voxes(V)
{
  assert(Voxes);
  side_len = len;
  f_side_len = len * 1.f;
  lod_dist = elements * len;

  // Origin вокселя, в котором расположена камера
  Location = { static_cast<int>(floorf(Pt.x / side_len)) * side_len,
               static_cast<int>(floorf(Pt.y / side_len)) * side_len,
               static_cast<int>(floorf(Pt.z / side_len)) * side_len };

  i3d P0 { Location.x - lod_dist,
           Location.y - lod_dist,
           Location.z - lod_dist };
  i3d P1 { Location.x + lod_dist,
           Location.y + lod_dist,
           Location.z + lod_dist };

  // Загрузка пространства вокруг точки Pt
  i3d vP {};
  for(vP.x = P0.x; vP.x<= P1.x; vP.x += side_len)
    for(vP.y = P0.y; vP.y<= P1.y; vP.y += side_len)
      for(vP.z = P0.z; vP.z<= P1.z; vP.z += side_len)
        Voxes->load(vP);
}


///
/// \brief space::recalc_borders
/// \details Перестроение границ активной области при перемещении камеры
///
void area::recalc_borders(const glm::vec3&)
{
  if(MovingDist.x > f_side_len)
  {
      MovingDist.x -= f_side_len;
      Location.x += side_len;
      redraw_borders_x(Location.x + lod_dist, Location.x - lod_dist - side_len);
  } else if(MovingDist.x < -f_side_len)
  {
      MovingDist.x += f_side_len;
      Location.x -= side_len;
      redraw_borders_x(Location.x - lod_dist, Location.x + lod_dist + side_len);
  }

  if(MovingDist.z > f_side_len)
  {
      MovingDist.z -= f_side_len;
      Location.z += side_len;
      redraw_borders_z(Location.z + lod_dist, Location.z - lod_dist - side_len);
  } else if(MovingDist.z < -f_side_len)
  {
      MovingDist.z += f_side_len;
      Location.z -= side_len;
      redraw_borders_z(Location.z - lod_dist, Location.z + lod_dist + side_len);
  }

  queue_release();
}


///
/// \brief area::redraw_borders_x
/// \details Перестроение границ LOD при перемещении по оси X
///
void area::redraw_borders_x(int add, int del)
{
  for(int y = -lod_dist; y <= lod_dist; y += side_len)
    for(int z = Location.z - lod_dist, max = Location.z + lod_dist; z <= max; z += side_len)
    {
      QueueWipe.push({del, y, z});
      QueueLoad.push({add, y, z});
    }
}


///
/// \brief area::redraw_borders_z
/// \details Перестроение границ LOD при перемещении по оси Z
///
void area::redraw_borders_z(int add, int del)
{
  for(int y = -lod_dist; y <= lod_dist; y += side_len)
    for(int x = Location.x - lod_dist, max = Location.x + lod_dist; x <= max; x += side_len)
    {
      QueueWipe.push({x, y, del});
      QueueLoad.push({x, y, add});
    }
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

} //namespace
