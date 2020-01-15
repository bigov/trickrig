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

//
/// \param length         // размер стороны вокселя текущего LOD
/// \param count          // количество вокселей до границы LOD
/// \param CameraLocation // Положение камеры в пространстве
/// \param Context        // OpenGL контекст назначенный текущему потоку
///
/// \details Создание отдельного потока обмена данными с базой
///
void db_control(std::mutex& m, wglfw* GLWindow, std::shared_ptr<glm::vec3> CameraLocation,
                GLuint vbo_id, GLsizeiptr vbo_size)
{
  GLWindow->gl_context_set_current();
  area Area {m, vbo_id, vbo_size};
  Area.load(CameraLocation);
  m.lock();   // На время загрузки сцены из БД заблокировать основной поток
  glFinish(); // синхронизация изменений между потоками
  m.unlock();

  while (render_indices >= 0) {
    if(!Area.recalc_borders())
    {
      std::this_thread::sleep_for(std::chrono::milliseconds(1));
    }
    else
    {
      m.lock();
      glFinish(); // синхронизация изменений между потоками
      m.unlock();
    }
  }

}

///
/// \brief area::area
/// \param length - длина стороны вокселя
/// \param count - число вокселей от камеры (или внутренней границы)
/// до внешней границы области
///
area::area (std::mutex& m, GLuint VBO_id, GLsizeiptr VBO_size): rVboAccess(m)
{
  side_len = size_v4;
  f_side_len = size_v4 * 1.f;
  lod_dist = border_dist_b4 * size_v4;

  VboCtrl = std::make_unique<vbo_ctrl> (GL_ARRAY_BUFFER, VBO_id, VBO_size);

  // Зарезервировать место для размещения числа сторон в соответствии с размером буфера VBO
  VboMap = std::unique_ptr<vbo_map[]> {new vbo_map[VBO_size/bytes_per_side]};

}


///
/// \brief area::load
/// \param CameraLocation
/// \details Загрузка пространства вокруг точки расположения камеры
///
void area::load(std::shared_ptr<glm::vec3> CameraLocation)
{
  ViewFrom = CameraLocation;

  mutex_viewfrom.lock();
  curr[XL] = ViewFrom->x;
  curr[ZL] = ViewFrom->z;
  mutex_viewfrom.unlock();

  memcpy(last, curr, sizeof (float) * sizeL);

  // Origin вокселя, в котором расположена камера
  origin[XL] = static_cast<int>(floorf(curr[XL] / side_len)) * side_len;
  origin[ZL] = static_cast<int>(floorf(curr[ZL] / side_len)) * side_len;

  int min_x = origin[XL] - lod_dist;
  int min_z = origin[ZL] - lod_dist;

  int max_x = origin[XL] + lod_dist;
  int max_z = origin[ZL] + lod_dist;

  for(int x = min_x; x<= max_x; x += side_len)
    for(int z = min_z; z<= max_z; z += side_len)
      load(x, z);
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
  curr[XL] = ViewFrom->x;
  curr[ZL] = ViewFrom->z;
  mutex_viewfrom.unlock();

  // Расстояние, на которое сместилась камера за время между вызовами
  move_dist[XL] += curr[XL] - last[XL];
  move_dist[ZL] += curr[ZL] - last[ZL];

  memcpy(last, curr, sizeof (float) * sizeL);

  if(move_dist[XL] > f_side_len)
  {
      move_dist[XL] -= f_side_len;
      origin[XL] += side_len;
      need_redraw_px = true;
  } else if(move_dist[XL] < -f_side_len)
  {
      move_dist[XL] += f_side_len;
      origin[XL] -= side_len;
      need_redraw_nx = true;
  }

  if(move_dist[ZL] > f_side_len)
  {
      move_dist[ZL] -= f_side_len;
      origin[ZL] += side_len;
      need_redraw_pz = true;
  } else if(move_dist[ZL] < -f_side_len)
  {
      move_dist[ZL] += f_side_len;
      origin[ZL] -= side_len;
      need_redraw_nz = true;
  }

  if(need_redraw_px) redraw_borders_x(origin[XL] + lod_dist, origin[XL] - lod_dist - side_len);
  if(need_redraw_nx) redraw_borders_x(origin[XL] - lod_dist, origin[XL] + lod_dist + side_len);
  if(need_redraw_pz) redraw_borders_z(origin[ZL] + lod_dist, origin[ZL] - lod_dist - side_len);
  if(need_redraw_nz) redraw_borders_z(origin[ZL] - lod_dist, origin[ZL] + lod_dist + side_len);

  return (need_redraw_px or need_redraw_pz or need_redraw_nx or need_redraw_nz);
}


///
/// \brief area::redraw_borders_x
/// \details Перестроение границ LOD при перемещении по оси X
///
void area::redraw_borders_x(int x_add, int x_del)
{
  for(int z = origin[ZL] - lod_dist, max = origin[ZL] + lod_dist; z <= max; z += side_len)
  {
    truncate(x_del, z);
    load(x_add, z);
  }
}


///
/// \brief area::redraw_borders_z
/// \details Перестроение границ LOD при перемещении по оси Z
///
void area::redraw_borders_z(int z_add, int z_del)
{
  for(int x = origin[XL] - lod_dist, max = origin[XL] + lod_dist; x <= max; x += side_len)
  {
    truncate(x, z_del);
    load(x, z_add);
  }
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

  GLsizeiptr vbo_addr;
  uchar n;
  uchar* data = nullptr;
  int y = 0;
  size_t offset = 0;
  size_t offset_max = VoxData.size() - sizeof_y - 1;
  while (offset < offset_max)                           // Если в блоке несколько воксов, то
  {                                                     // последовательно передать все в VBO.
    memcpy(&y, VoxData.data() + offset, sizeof_y);      // Координата "y" вокса, записанная в блоке данных
    offset += sizeof_y;
    std::bitset<6> m (VoxData[offset]);                 // Маcка видимых сторон
    offset += 1;
    data = VoxData.data()+ offset;
    n = m.count();                                      // Число видимых сторон текущего вокса
    while (n > 0)                                       // Все стороны передать в VBO.
    {
      n--;
      rVboAccess.lock();
      vbo_addr = VboCtrl->append(data, bytes_per_side); // Добавить данные стороны в VBO
      rVboAccess.unlock();
      VboMap[vbo_addr/bytes_per_side] = {x, y, z, n};   // Запомнить положение блока данных стороны
      render_indices.fetch_add(indices_per_side);       // Увеличить число точек рендера
      data += bytes_per_side;                           // Переключить указатель на начало следующей стороны
    }
    offset += m.count() * bytes_per_side;               // Перейти к началу блока данных следующего вокса
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
      rVboAccess.lock();
      moved_from = VboCtrl->remove(dest, bytes_per_side); // адрес хвоста VBO (данными отсюда
      rVboAccess.unlock();                                // перезаписываются данные по адресу "dest")
      // Если c адреса "free" на "dest" данные были перенесены, то обновить координты Origin
      if (moved_from != dest) VboMap[id] = VboMap[moved_from/bytes_per_side];
      // Если free == dest, то удаляемый блок данных был в конце VBO и просто "отбрасывается"
      render_indices.fetch_sub(indices_per_side); // уменьшаем число точек рендера
    }
  }
}

} //namespace
