/**
 *
 * file: area.cpp
 *
 * Класс управления элементами области 3D пространства
 *
 */

#include "area.hpp"
#include <cfenv>

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
void db_control(std::shared_ptr<trgl> OpenGLContext,
                std::shared_ptr<glm::vec3> CameraLocation,
                GLuint vbo_id, GLsizeiptr vbo_size)
{
  log_mtx.lock();
  std::clog << "The db-thread is started" << std::endl;
  log_mtx.unlock();

  OpenGLContext->thread_enable();
  area Area {vbo_id, vbo_size};
  Area.init(std::move(CameraLocation));
  vbo_mtx.lock();
  glFinish();       // синхронизация изменений между потоками после загрузки данных
  vbo_mtx.unlock();

  log_mtx.lock();
  std::clog << "VBO filling is completed" << std::endl;
  log_mtx.unlock();


  while (render_indices >= 0)
    if(!Area.recalc_borders())
      std::this_thread::sleep_for(std::chrono::milliseconds(1));

  log_mtx.lock();
  std::clog << std::endl << "The db-thread is finished" << std::endl;
  log_mtx.unlock();

}

///
/// \brief area::area
/// \param length - длина стороны вокселя
/// \param count - число вокселей от камеры (или внутренней границы)
/// до внешней границы области
///
area::area (GLuint VBO_id, GLsizeiptr VBO_size)
{
  side_len = size_v4;
  f_side_len = size_v4 * 1.f;
  lod_dist = border_dist_b4 * size_v4;

  VboCtrl = std::make_unique<vbo_ctrl> (GL_ARRAY_BUFFER, VBO_id, VBO_size);

  // Зарезервировать место для размещения числа сторон в соответствии с размером буфера VBO
  VboMap = std::unique_ptr<vbo_map[]> {new vbo_map[VBO_size/bytes_per_face]};

}


///
/// \brief area::load
/// \param CameraLocation
/// \details Загрузка пространства вокруг точки расположения камеры
///
void area::init(std::shared_ptr<glm::vec3> CameraLocation)
{
  ViewFrom = CameraLocation;

  view_mtx.lock();
  curr[XL] = ViewFrom->x;
  curr[ZL] = ViewFrom->z;
  view_mtx.unlock();

  memcpy(last, curr, sizeof (float) * sizeL);

  // Origin вокселя, в котором расположена камера
  fesetround(FE_DOWNWARD);
  origin[XL] = rint(curr[XL] / side_len) * side_len; //static_cast<int>(floorf(curr[XL] / side_len)) * side_len;
  origin[ZL] = rint(curr[ZL] / side_len) * side_len; //static_cast<int>(floorf(curr[ZL] / side_len)) * side_len;

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
  change_control();

  bool redrawed = false;
  view_mtx.lock();
  curr[XL] = ViewFrom->x;
  curr[ZL] = ViewFrom->z;
  view_mtx.unlock();

  // Расстояние, на которое сместилась камера за время между вызовами
  move_dist[XL] += curr[XL] - last[XL];
  move_dist[ZL] += curr[ZL] - last[ZL];

  memcpy(last, curr, sizeof (float) * sizeL);

  if(move_dist[XL] > f_side_len)
  {
      move_dist[XL] -= f_side_len;
      origin[XL] += side_len;
      redraw_borders_x(origin[XL] + lod_dist, origin[XL] - lod_dist - side_len);
      redrawed = true;
  } else if(move_dist[XL] < -f_side_len)
  {
      move_dist[XL] += f_side_len;
      origin[XL] -= side_len;
      redraw_borders_x(origin[XL] - lod_dist, origin[XL] + lod_dist + side_len);
      redrawed = true;
  }

  if(move_dist[ZL] > f_side_len)
  {
      move_dist[ZL] -= f_side_len;
      origin[ZL] += side_len;
      redraw_borders_z(origin[ZL] + lod_dist, origin[ZL] - lod_dist - side_len);
      redrawed = true;
  } else if(move_dist[ZL] < -f_side_len)
  {
      move_dist[ZL] += f_side_len;
      origin[ZL] -= side_len;
      redraw_borders_z(origin[ZL] - lod_dist, origin[ZL] + lod_dist + side_len);
      redrawed = true;
  }

  if(redrawed)
  {
    vbo_mtx.lock();
    glFinish();
    vbo_mtx.unlock();
  }
  return redrawed;
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
/// \brief area::change_control
/// \return
/// \details анализ значения глобальной переменной click_side_vertex_id, в которой
/// находится старший из индексов вершин, образующих сторону, по которой был
/// выполнен клик мышкой;
/// - так как это число не может быть меньше количества вершин на одну сторону,
///   то нулевое значение означает, что клика не было;
/// - так как значение индекса в VBO всегда положительное, то отрицательое значение
///   вызывает функцию удаления вокса, а положительное - добавление.
///
void area::change_control(void)
{
  if(click_side_vertex_id == 0) return;

  int sign = 1;
  if(click_side_vertex_id < 0) sign = -1;

  int vertex_id = click_side_vertex_id * sign;
  click_side_vertex_id.store(0);

  auto V = VboMap[vertex_id / vertices_per_face];
  if(sign > 0) vox_append(V);
  else vox_remove(V);

  vbo_mtx.lock();
  glFinish();
  glFlush();
  vbo_mtx.unlock();
}


///
/// \brief area::vox_append
/// \param side
/// \param data
///
void area::vox_append(const vbo_map& V)
{
  auto P = i3d_near( {V.x, V.y, V.z}, V.face_id, side_len );

#ifndef NDEBUG
  int id = V.face_id;
  std::clog << std::endl << __PRETTY_FUNCTION__ << std::endl
            << "append Vox on " << V.x << "," << V.y << "," << V.z << ",f" << id
            << " -> " << P.x << "," << P.y << "," << P.z << std::endl;
#endif

  truncate(P.x, P.z);       // Убрать колонки из рендера
  truncate(P.x + side_len, P.z);
  truncate(P.x - side_len, P.z);  //    #
  truncate(P.x, P.z + side_len);  //   ###
  truncate(P.x, P.z - side_len);  //    #

  cfg::DataBase.vox_append(P.x, P.y, P.z, side_len); // Внести изменения в БД

  load(P.x, P.z);           // Загрузить данные в рендер
  load(P.x + side_len, P.z);
  load(P.x - side_len, P.z);
  load(P.x, P.z + side_len);
  load(P.x, P.z - side_len);
}


///
/// \brief area::vox_remove
/// \param side
/// \param data
///
void area::vox_remove(const vbo_map& V)
{
#ifndef NDEBUG
  std::clog << std::endl << __PRETTY_FUNCTION__ << std::endl
            << "remove Vox from " << V.x << "," << V.y << "," << V.z << std::endl;
#endif

  truncate(V.x, V.z);          // Убрать колонки из рендера
  truncate(V.x + side_len, V.z);
  truncate(V.x - side_len, V.z);
  truncate(V.x, V.z + side_len);
  truncate(V.x, V.z - side_len);

  cfg::DataBase.vox_delete(V.x, V.y, V.z, side_len); // Внести изменения в БД

  load(V.x + side_len, V.z);
  load(V.x - side_len, V.z);
  load(V.x, V.z + side_len);
  load(V.x, V.z - side_len);
  load(V.x, V.z);              // Загрузить данные в рендер
}


///
/// \brief area::load
/// \param P0
/// \details Загрузить из базы данных в рендер пакет воксов
///
void area::load(int x, int z)
{
  auto DataPack = cfg::DataBase.area_load(x, z, side_len);
  for( auto& V: DataPack.Voxes )
  {
    for( auto& Face: V.Faces )
    {
      vbo_mtx.lock();
      auto vbo_addr = VboCtrl->append(Face.data() + 1, bytes_per_face);
      render_indices.fetch_add(indices_per_face);
      vbo_mtx.unlock();
      // Запомнить положение блока данных в VBO, координаты вокса и индекс поверхности
      VboMap[vbo_addr/bytes_per_face] = { x, V.y, z, Face[0] }; // По адресу [0] находится id поверхности
    }
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
  if (render_indices < indices_per_face) return;
  GLsizeiptr dest, moved_from;

  size_t id = render_indices/indices_per_face;
  while(id > 0)
  {
    id -= 1;
    if((VboMap[id].x == x) and (VboMap[id].z == z)) // Данные вокса есть в GPU
    {
      dest = id * bytes_per_face;                     // адрес удаляемого блока данных
      vbo_mtx.lock();
      moved_from = VboCtrl->remove(dest, bytes_per_face); // адрес хвоста VBO (данными отсюда
      vbo_mtx.unlock();                                // перезаписываются данные по адресу "dest")
      // Если c адреса "free" на "dest" данные были перенесены, то обновить координты Origin
      if (moved_from != dest) VboMap[id] = VboMap[moved_from/bytes_per_face];
      // Если free == dest, то удаляемый блок данных был в конце VBO и просто "отбрасывается"
      render_indices.fetch_sub(indices_per_face); // уменьшаем число точек рендера
    }
  }
}

} //namespace
