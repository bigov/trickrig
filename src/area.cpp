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
void area::init (std::shared_ptr<voxesdb> V, int len, int elements, std::shared_ptr<glm::vec3> CameraLocation)
{
  Voxes = V;
  side_len = len;
  f_side_len = len * 1.f;
  lod_dist = elements * len;
  ViewFrom = CameraLocation;

  // Origin вокселя, в котором расположена камера
  Location = { static_cast<int>(floorf(ViewFrom->x / side_len)) * side_len,
               static_cast<int>(floorf(ViewFrom->y / side_len)) * side_len,
               static_cast<int>(floorf(ViewFrom->z / side_len)) * side_len };

  i3d P0 { Location.x - lod_dist,
           Location.y - lod_dist,
           Location.z - lod_dist };
  i3d P1 { Location.x + lod_dist,
           Location.y + lod_dist,
           Location.z + lod_dist };

  // Загрузка пространства вокруг точки расположения камеры
  i3d vP {};
//  std::lock_guard<std::mutex> Hasp{mutex_loading};

  for(vP.x = P0.x; vP.x<= P1.x; vP.x += side_len)
      for(vP.z = P0.z; vP.z<= P1.z; vP.z += side_len)
      {
        Voxes->load(vP.x, vP.z);
      }
  //glFinish();
}


///
/// \brief area::operator()
/// \param V          // адрес базы данных контроля вокселей
/// \param len        // размер стороны вокселя текущего LOD
/// \param elements   // количество вокселей до границы LOD
/// \param Pt         // Положение камеры в пространстве
/// \param Context    // OpenGL контекст назначенный текущему потоку
///
/// \details Данный метод класса вызывается при создании потока
///
void area::operator() (std::shared_ptr<voxesdb> V, int len, int elements, std::shared_ptr<glm::vec3> CameraLocation, GLFWwindow* Context)
{
  glfwMakeContextCurrent(Context);
  init(V, len, elements, CameraLocation);

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
bool area::recalc_borders (void)
{
  bool
    need_redraw_px = false,
    need_redraw_pz = false,
    need_redraw_nx = false,
    need_redraw_nz = false;

  mutex_viewfrom.lock();
  curr[0] = ViewFrom->x;
  curr[1] = ViewFrom->y;
  curr[2] = ViewFrom->z;
  mutex_viewfrom.unlock();

  // Расстояние, на которое сместилась камера за время между вызовами
  for(int i = 0; i < 3; i++) move_dist[i] += curr[i] - last[i];
  memcpy(last, curr, sizeof (float) * 3);

  if(move_dist[0] > f_side_len)
  {
      move_dist[0] -= f_side_len;
      Location.x += side_len;
      need_redraw_px = true;
  } else if(move_dist[0] < -f_side_len)
  {
      move_dist[0] += f_side_len;
      Location.x -= side_len;
      need_redraw_nx = true;
  }

  if(move_dist[2] > f_side_len)
  {
      move_dist[2] -= f_side_len;
      Location.z += side_len;
      need_redraw_pz = true;
  } else if(move_dist[2] < -f_side_len)
  {
      move_dist[2] += f_side_len;
      Location.z -= side_len;
      need_redraw_nz = true;
  }

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
void area::redraw_borders_x(int x_add, int x_del)
{
  for(int z = Location.z - lod_dist, max = Location.z + lod_dist; z <= max; z += side_len)
  {
    Voxes->vbo_truncate(x_del, z);
    Voxes->load(x_add, z);
  }
}


///
/// \brief area::redraw_borders_z
/// \details Перестроение границ LOD при перемещении по оси Z
///
void area::redraw_borders_z(int z_add, int z_del)
{
  for(int x = Location.x - lod_dist, max = Location.x + lod_dist; x <= max; x += side_len)
  {
    Voxes->vbo_truncate(x, z_del);
    Voxes->load(x, z_add);
  }
}

} //namespace
