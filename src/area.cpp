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
/// \brief cross
/// \param s
/// \return номер стороны, противоположной указанной в параметре
///
u_char side_opposite(u_char s)
{
  switch (s)
  {
    case SIDE_XP: return SIDE_XN;
    case SIDE_XN: return SIDE_XP;
    case SIDE_YP: return SIDE_YN;
    case SIDE_YN: return SIDE_YP;
    case SIDE_ZP: return SIDE_ZN;
    case SIDE_ZN: return SIDE_ZP;
    default: return UCHAR_MAX;
  }
}


///
/// \brief area::area
/// \param length - длина стороны вокселя
/// \param count - число вокселей от камеры (или внутренней границы)
/// до внешней границы области
///
area::area(int side_length, int count_elements_to_border, const glm::vec3& ViewFrom, vbo_ext* V)
{
  pVBO = V;
  vox_side_len = side_length;
  lod_dist_far = count_elements_to_border * side_length;

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
  for(vP.z = P0.z; vP.z<= P1.z; vP.z += vox_side_len) vox_load(vP);

}


///
/// \brief vox_buffer::get_render_indices
/// \return
///
u_int area::get_render_indices(void)
{
  return render_indices;
}


///
/// \brief space::recalc_borders
/// \details Перестроение границ активной области при перемещении камеры
///
/// TODO? (на случай притормаживания - если прыгать камерой туда-сюда через
/// границу запуска перерисовки границ) можно процедуры "redraw_borders_?"
/// разбить по две части - вперед/назад.
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
  if(id > (render_indices/indices_per_side) * bytes_per_side) return;

  GLsizeiptr offset = (id/vertices_per_side) * bytes_per_side;
  vox* pVox = vox_by_vbo(offset);
  if (nullptr == pVox) return;
  i3d P0 = i3d_near(pVox->Origin, pVox->side_id_by_offset(offset)); // координаты нового вокса

  for (u_char side = 0; side < SIDES_COUNT; ++side) // Временно убрать из рендера воксы
    vox_wipe(vox_by_i3d(i3d_near(P0, side)));   // вокруг точки добавления вокса

  // Добавить вокс в БД и в рендер VBO
  vox* V = vox_add_in_db(P0);
  recalc_vox_visibility(V);
  vox_draw(V);

  for (u_char side = 0; side < SIDES_COUNT; ++side) // Вернуть в рендер соседние воксы
    vox_draw(vox_by_i3d(i3d_near(P0, side)));
}


///
/// \brief vox_buffer::remove
/// \param i - порядковый номер группы данных из буфера
///
/// \details Удаление вокса
///
void area::remove(u_int i)
{
  if(i > (render_indices/indices_per_side) * bytes_per_side) return;
  GLsizeiptr offset = (i/vertices_per_side) * bytes_per_side; // по номеру группы - адрес смещения в VBO
  vox* V = vox_by_vbo(offset);
  if(nullptr == V) ERR("Error: try to remove unexisted vox");
  i3d P0 = V->Origin;

  for (u_char side = 0; side < SIDES_COUNT; ++side) // Временно убрать из рендера воксы вокруг
    vox_wipe(vox_by_i3d(i3d_near(P0, side)));

  vox_wipe(V);                   // Выделенный вокс убрать из рендера
  cfg::DataBase.erase_vox(V);    // удалить из базы данных,
  vox_unload(P0);                // удалить из буфера
  recalc_around_visibility(P0);  // пересчитать видимость вокселей вокруг

  for (u_char side = 0; side < SIDES_COUNT; ++side) // Вернуть в рендер соседние воксели
    vox_draw(vox_by_i3d(i3d_near(P0, side)));
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
      vox_load(QueueLoad.front()); // Загрузка данных имеет приоритет. Пока вся очередь
      QueueLoad.pop();                        // загрузки не очистится, очередь выгрузки ждет.
      continue;
    }

    // Удаление из памяти данных тех элементов, которые в результате перемещения
    // камеры вышли за границу отображения текущего LOD
    if(!QueueWipe.empty())
    {
      vox_unload(QueueWipe.front());
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


///
/// \brief vox_buffer::i3d_near
/// \param P
/// \param s
/// \param l
/// \return
///
///  Координаты опорной точки соседнего вокселя относительно указанной стороны
///
i3d area::i3d_near(const i3d& P, u_char side)
{
  switch (side) {
    case SIDE_XP: return i3d{ P.x + vox_side_len, P.y, P.z };
    case SIDE_XN: return i3d{ P.x - vox_side_len, P.y, P.z };
    case SIDE_YP: return i3d{ P.x, P.y + vox_side_len, P.z };
    case SIDE_YN: return i3d{ P.x, P.y - vox_side_len, P.z };
    case SIDE_ZP: return i3d{ P.x, P.y, P.z + vox_side_len };
    case SIDE_ZN: return i3d{ P.x, P.y, P.z - vox_side_len };
    default:      return P;
  }
}


///
/// \brief vox_buffer::add_vox
/// \param P
/// \return vox*
///
/// \details В указанной 3D точке генерирует новый вокс c пересчетом
/// видимости своих сторон и окружающих воксов и вносит его в базу данных
///
vox* area::vox_add_in_db(const i3d& P)
{
  auto pVox = cfg::DataBase.get_vox(P, vox_side_len);
  if (nullptr == pVox)
  {
    pVox = std::make_unique<vox> (P, vox_side_len);
    cfg::DataBase.save_vox(pVox.get());
  }
  MemArea.push_back(std::move(pVox));
  return MemArea.back().get();
}


///
/// \brief vox_buffer::vox_load
/// \param P0
/// \details Загрузить вокс из базы данных в буфер и в рендер
///
void area::vox_load(const i3d& P0)
{
  auto pVox = cfg::DataBase.get_vox(P0, vox_side_len);

  if(nullptr != pVox)
  {
    MemArea.push_back(std::move(pVox));
    vox* V = MemArea.back().get();

    for (u_char side = 0; side < SIDES_COUNT; ++side) // Временно убрать из рендера воксы
      vox_wipe(vox_by_i3d(i3d_near(P0, side)));   // вокруг точки добавления вокса

    recalc_vox_visibility(V); // пересчитать видимость сторон
    vox_draw(V);              // добавить в рендер

    for (u_char side = 0; side < SIDES_COUNT; ++side) // Вернуть в рендер соседние воксы
      vox_draw(vox_by_i3d(i3d_near(P0, side)));
  }
}


///
/// \brief area::vox_unload
/// \param P0
/// \details Выгрузить данные вокса из памяти, и убрать из рендера
///
void area::vox_unload(const i3d& P0)
{
  auto it = std::find_if(MemArea.begin(), MemArea.end(),
                         [&P0](const auto& V){ return V->Origin == P0; });
  if(it == MemArea.end()) return; // в этой точке вокса нет
  vox* V = it->get();
  if(V->in_vbo) vox_wipe(V);
  MemArea.erase(it);
}


///
/// \brief vox_buffer::vox_draw
///
/// \details Добавление в графический буфер элементов
///
/// Координаты вершин хранятся в нормализованом виде, поэтому перед записью
/// в VBO данные копируются во временный кэш, где координаты пересчитываются
/// с учетом координат вокселя, после чего записываются в VBO. Адрес
/// размещения данных в VBO для каждой стороны вокселя сохраняется в параметрах вокселя.
///
void area::vox_draw(vox* pVox)
{
  if(nullptr == pVox) return;
#ifndef NDEBUG
  if(pVox->in_vbo) ERR("vox_draw: duplicate draw");
#endif

  GLsizeiptr vbo_addr = 0;
  GLfloat buffer[digits_per_side];

  // каждая сторона вокса обрабатывается отдельно
  for(u_char side_id = 0; side_id < SIDES_COUNT; ++side_id)
  {
    if(!pVox->side_fill_data(side_id, buffer)) continue;
    vbo_addr = pVBO->append(buffer, bytes_per_side);
    pVox->offset_write(side_id, vbo_addr);     // запомнить адрес смещения стороны в VBO
    render_indices += indices_per_side;        // увеличить число точек рендера
  }
  pVox->in_vbo = true;
}


///
/// \brief vox_buffer::vox_wipe
/// \param vox* pVox
/// \details Убрать вокс из рендера
///
void area::vox_wipe(vox* pVox)
{
  if(nullptr == pVox) return;
  if(!pVox->in_vbo) return;

  // Данные каждой из сторон перезаписываются данными воксов с хвоста VBO
  for(u_char side_id = 0; side_id < SIDES_COUNT; ++side_id)
  {
    if(!pVox->is_visible(side_id)) continue;      // если сторона невидима, то пропустить цикл
    GLsizeiptr dest = pVox->offset_read(side_id); // адрес освобождаемого блока данных
    pVox->offset_write(side_id, -1);              // чтобы функция поиска больше не нвходила этот вокс

    // free - новый адрес границы VBO (отсюда данные были перенесены по адресу "dest")
    GLsizeiptr free = pVBO->remove(dest, bytes_per_side); // переписать в VBO блок данных по адресу "dest"
    if(free == 0)
    { // VBO пустой
      render_indices = 0;
      pVox->in_vbo = false;
      return;
    }

    if (free != dest)  // на "dest" были перенесены данные c адреса "free".
    {
      vox* V = vox_by_vbo(free); // Найти вокс, чьи данные были перенесены
      if(nullptr == V) ERR("vox_wipe: find_in_buffer(" + std::to_string(free) + ")");
      V->offset_replace(free, dest); // скорректировать адрес размещения данных
    }
    // Если free == dest, то удаляемый блок данных был в конце VBO,
    // поэтому только уменьшаем число точек рендера
    render_indices -= indices_per_side;
  }
  pVox->in_vbo = false;
}


///
/// \brief vox_buffer::vox_by_i3d
/// \param P0
/// \return
/// \details Поиск в буфере вокса по его координатам
///
vox* area::vox_by_i3d(const i3d& P0)
{
  auto it = std::find_if(MemArea.begin(), MemArea.end(),
                         [&P0](const auto& V){ return V->Origin == P0; });
  if (it == MemArea.end()) return nullptr;
  else return it->get();
}


///
/// \brief vox_buffer::vox_by_vbo
/// \param addr
/// \return
/// \details Поиск в буфере вокса, у которого сторона размещена в VBO по указанному адресу
vox* area::vox_by_vbo(GLsizeiptr addr)
{
  auto it = std::find_if( MemArea.begin(), MemArea.end(),
    [&addr](auto& V)
    {
      if(!V->in_vbo) return false;
      return V->side_id_by_offset(addr) < SIDES_COUNT;
    }
  );
  if (it == MemArea.end()) return nullptr;
  else return it->get();
}


///
/// \brief vox_buffer::recalc_vox_visibility
/// \param R0
/// \details Пересчет видимости сторон вокса и его соседей вокруг него
///
void area::recalc_vox_visibility(vox* pVox)
{
  if (nullptr == pVox) return;

  // С каждой стороны вокса проверить наличие соседа
  for (u_char side = 0; side < SIDES_COUNT; ++side)
  {
    vox* pNear = vox_by_i3d(i3d_near(pVox->Origin, side));
    if (nullptr != pNear)                      // Если есть соседний,
    {                                          // то соприкасающиеся
      pVox->visible_off(side);                 // стороны невидимы
      pNear->visible_off(side_opposite(side)); // у обоих воксов.
    } else
    {
      pVox->visible_on(side);                  // Иначе - сторона видимая.
    }
  }
}


///
/// \brief vox_buffer::recalc_around_visibility
/// \param P0
/// \details Пересчет видимости воксов вокруг опорной точки
///
void area::recalc_around_visibility(i3d P0)
{
  vox* pVox = vox_by_i3d(P0);
  if (nullptr != pVox)
  {                              // Если в этой точке есть вокс,
    recalc_vox_visibility(pVox); // то вызвать пересчет видимости
    return;                      // его сторон и выйти.
  }

  // Если в указанной точке пусто, то у воксов вокруг нее
  // надо включить видимость прилегающих сторон
  for (u_char side = 0; side < SIDES_COUNT; ++side)
  {
    pVox = vox_by_i3d(i3d_near(P0, side));
    if (nullptr != pVox) pVox->visible_on(side_opposite(side));
  }
}


} //namespace
