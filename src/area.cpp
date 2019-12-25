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
void area::init (int len, int elements, std::shared_ptr<glm::vec3> CameraLocation)
{
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

  std::lock_guard<std::mutex> Hasp{mutex_voxes_db};
  for(vP.x = P0.x; vP.x<= P1.x; vP.x += side_len) for(vP.z = P0.z; vP.z<= P1.z; vP.z += side_len)
    load(vP.x, vP.z);
  glFinish(); // синхронизация изменений между потоками
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
void area::operator() (int len, int elements, std::shared_ptr<glm::vec3> CameraLocation,
                       GLFWwindow* Context, GLenum buf_type, GLuint buf_id, GLsizeiptr buf_size)
{
  VBOctrl = std::make_unique<vbo_ctrl> (buf_type, buf_id, buf_size);

  // Зарезервировать место в соответствии с размером буфера VBO
  VboMap = std::unique_ptr<vbo_map[]> {new vbo_map[buf_size/bytes_per_side]};


  glfwMakeContextCurrent(Context);
  init(len, elements, CameraLocation);

  while (render_indices >= 0) {
    if(!recalc_borders()) std::this_thread::sleep_for(std::chrono::milliseconds(1));
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
    mutex_voxes_db.lock();
    truncate(x_del, z);
    mutex_voxes_db.unlock();
    std::this_thread::sleep_for(std::chrono::microseconds(1));
    mutex_voxes_db.lock();
    load(x_add, z);
    mutex_voxes_db.unlock();
  }
  glFinish(); // синхронизация изменений между потоками
}


///
/// \brief area::redraw_borders_z
/// \details Перестроение границ LOD при перемещении по оси Z
///
void area::redraw_borders_z(int z_add, int z_del)
{
  for(int x = Location.x - lod_dist, max = Location.x + lod_dist; x <= max; x += side_len)
  {
    mutex_voxes_db.lock();
    truncate(x, z_del);
    mutex_voxes_db.unlock();
    std::this_thread::sleep_for(std::chrono::microseconds(1));
    mutex_voxes_db.lock();
    load(x, z_add);
    mutex_voxes_db.unlock();
  }
  glFinish(); // синхронизация изменений между потоками
}


///
/// \brief voxesdb::vox_load
/// \param P0
/// \details Загрузить вокс из базы данных в рендер
///
void area::load(int x, int z)
{
  // Загрузка из БД
  auto VoxData = cfg::DataBase.load_vox_data(x, z);
  if(VoxData.empty()) return;

  int y = 0;
  size_t offset = 0;
  size_t offset_max = VoxData.size() - sizeof_y - 1;
  while (offset < offset_max)
  {
    memcpy(&y, VoxData.data() + offset, sizeof_y);  // Координата "y"
    offset += sizeof_y;
    std::bitset<6> m(VoxData[offset]);              // Маcка видимых сторон
    offset += 1;

    uchar* data = VoxData.data()+ offset;

    GLsizeiptr vbo_addr = 0;
    uchar n = m.count();
    while (n > 0)
    {                  //TODO: настроить передачу информации в VBO за одно обращение
      n--;
      vbo_addr = VBOctrl->append(data, bytes_per_side); // добавить данные стороны в VBO
      VboMap[vbo_addr/bytes_per_side] = {x, y, z, n};   // запомнить положение блока данных
      render_indices.fetch_add(indices_per_side);       // увеличить число точек рендера
      data += bytes_per_side;                           // переключить на следующую сторону
    }
    offset += m.count() * bytes_per_side;               // Перейти к следующему блоку
  }
}


///
/// \brief area::truncate
/// \param x
/// \param z
/// \details Удаление данных из VBO
///
void area::truncate(int x, int z)
{
  if (render_indices < indices_per_side) return;
  GLsizeiptr dest, moved_from;

  size_t id = render_indices/indices_per_side;
  while(id > 0)
  {
    id -= 1;
    if((VboMap[id].x == x) and (VboMap[id].z == z)) // Данные вокса есть в GPU
    {
      dest = id * bytes_per_side;                     // адрес удаляемого блока данных
      moved_from = VBOctrl->remove(dest, bytes_per_side); // адрес хвоста VBO (данными отсюда
                                                      // перезаписываются данные по адресу "dest")
      // Если c адреса "free" на "dest" данные были перенесены, то обновить координты Origin
      if (moved_from != dest) VboMap[id] = VboMap[moved_from/bytes_per_side];
      // Если free == dest, то удаляемый блок данных был в конце VBO и просто "отбрасывается"
      render_indices.fetch_sub(indices_per_side); // уменьшаем число точек рендера
    }
  }
}

} //namespace
