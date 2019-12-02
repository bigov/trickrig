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
void area::init(const glm::vec3& Pt)
{
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
  std::lock_guard<std::mutex> Hasp{mutex_loading};

  for(vP.x = P0.x; vP.x<= P1.x; vP.x += side_len)
    for(vP.y = P0.y; vP.y<= P1.y; vP.y += side_len)
      for(vP.z = P0.z; vP.z<= P1.z; vP.z += side_len)
      {
        Voxes->load(vP);
      }
}


///
/// \brief area::operator ()
/// \param V
/// \param len
/// \param elements
/// \param Pt
/// \param Sh
///
void area::operator() (std::shared_ptr<voxesdb> V, int len, int elements, const glm::vec3& Pt, GLFWwindow* Context)
{
  auto t0 = std::chrono::milliseconds(1);

  Voxes = V;
  side_len = len;
  f_side_len = len * 1.f;
  lod_dist = elements * len;

  glfwMakeContextCurrent(Context);
  std::this_thread::sleep_for(t0);

  init(Pt);

  //mutex_mdist.lock();
  //MovingDist = {0.f, 0.f, 0.f};
  //mutex_mdist.unlock();

  while (nullptr != Voxes) {
    //recalc_borders();
    std::this_thread::sleep_for(t0);
  }
}


///
/// \brief space::recalc_borders
/// \details Перестроение границ активной области при перемещении камеры
///
void area::recalc_borders(void)
{
  bool need_redraw_px = false, need_redraw_pz = false;
  bool need_redraw_nx = false, need_redraw_nz = false;

  mutex_mdist.lock();
  if(MovingDist.x > f_side_len)
  {
      // DEBUG -->
      std::cout << MovingDist.x;
      // <-- DEBUG

      MovingDist.x -= f_side_len;
      Location.x += side_len;
      need_redraw_px = true;
  } else if(MovingDist.x < -f_side_len)
  {
      // DEBUG -->
      std::cout << MovingDist.x;
      // <-- DEBUG

      MovingDist.x += f_side_len;
      Location.x -= side_len;
      need_redraw_nx = true;
  }

  if(MovingDist.z > f_side_len)
  {
      // DEBUG -->
      std::cout << MovingDist.z;
      // <-- DEBUG

      MovingDist.z -= f_side_len;
      Location.z += side_len;
      need_redraw_pz = true;
  } else if(MovingDist.z < -f_side_len)
  {
      // DEBUG -->
      std::cout << MovingDist.z;
      // <-- DEBUG

      MovingDist.z += f_side_len;
      Location.z -= side_len;
      need_redraw_nz = true;
  }
  mutex_mdist.unlock();

  if(need_redraw_px) redraw_borders_x(Location.x + lod_dist, Location.x - lod_dist - side_len);
  if(need_redraw_nx) redraw_borders_x(Location.x - lod_dist, Location.x + lod_dist + side_len);
  if(need_redraw_pz) redraw_borders_z(Location.z + lod_dist, Location.z - lod_dist - side_len);
  if(need_redraw_nz) redraw_borders_z(Location.z - lod_dist, Location.z + lod_dist + side_len);
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
      Voxes->unload({del, y, z});
      Voxes->load({add, y, z});
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
      Voxes->unload({x, y, del});
      Voxes->load({x, y, add});
    }
}

} //namespace
