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
void area::init(std::shared_ptr<voxesdb> V, int len, int elements, const glm::vec3& Pt)
{
  Voxes = V;
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
  std::lock_guard<std::mutex> Hasp{mutex_loading};

  for(vP.x = P0.x; vP.x<= P1.x; vP.x += side_len)
    for(vP.y = P0.y; vP.y<= P1.y; vP.y += side_len)
      for(vP.z = P0.z; vP.z<= P1.z; vP.z += side_len)
      {
        Voxes->load(vP);
      }
  //glFinish();
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
  glfwMakeContextCurrent(Context);
  init(V, len, elements, Pt);

  auto t0 = std::chrono::milliseconds(1);

  while (nullptr != Voxes) {
    if(!recalc_borders())
      std::this_thread::sleep_for(t0); // чтобы не грузить процессор
  }
}


///
/// \brief space::recalc_borders
/// \details Перестроение границ активной области при перемещении камеры
///
bool area::recalc_borders(void)
{
  bool
    need_redraw_px = false,
    need_redraw_pz = false,
    need_redraw_nx = false,
    need_redraw_nz = false;

  mutex_mdist.lock();
  if(MovingDist.x > f_side_len)
  {
      MovingDist.x -= f_side_len;
      Location.x += side_len;
      need_redraw_px = true;
  } else if(MovingDist.x < -f_side_len)
  {
      MovingDist.x += f_side_len;
      Location.x -= side_len;
      need_redraw_nx = true;
  }

  if(MovingDist.z > f_side_len)
  {
      MovingDist.z -= f_side_len;
      Location.z += side_len;
      need_redraw_pz = true;
  } else if(MovingDist.z < -f_side_len)
  {
      MovingDist.z += f_side_len;
      Location.z -= side_len;
      need_redraw_nz = true;
  }
  mutex_mdist.unlock();

  if(need_redraw_px) redraw_borders_x(Location.x + lod_dist, Location.x - lod_dist - side_len);
  if(need_redraw_nx) redraw_borders_x(Location.x - lod_dist, Location.x + lod_dist + side_len);
  if(need_redraw_pz) redraw_borders_z(Location.z + lod_dist, Location.z - lod_dist - side_len);
  if(need_redraw_nz) redraw_borders_z(Location.z - lod_dist, Location.z + lod_dist + side_len);

  return (need_redraw_px or need_redraw_pz or need_redraw_nx or need_redraw_nz);
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
