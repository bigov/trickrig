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
    case SIDE_XP: return SIDE_XN; break;
    case SIDE_XN: return SIDE_XP; break;
    case SIDE_YP: return SIDE_YN; break;
    case SIDE_YN: return SIDE_YP; break;
    case SIDE_ZP: return SIDE_ZN; break;
    case SIDE_ZN: return SIDE_ZP; break;
    default: return UCHAR_MAX;
  }
}


///
/// \brief area::area
/// \param length - длина стороны вокселя
/// \param count - число вокселей от камеры (или внутренней границы)
/// до внешней границы области
///
area::area(int length, int count)
{
  voxel_size = length;
  area_width = count * length;
}


///
/// \brief i3d_shift
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
    case SIDE_XP:
      return i3d{ P.x + voxel_size, P.y, P.z };
      break;
    case SIDE_XN:
      return i3d{ P.x - voxel_size, P.y, P.z };
      break;
    case SIDE_YP:
      return i3d{ P.x, P.y + voxel_size, P.z };
      break;
    case SIDE_YN:
      return i3d{ P.x, P.y - voxel_size, P.z };
      break;
    case SIDE_ZP:
      return i3d{ P.x, P.y, P.z + voxel_size };
      break;
    case SIDE_ZN:
      return i3d{ P.x, P.y, P.z - voxel_size };
      break;
    default:
      return P;
  }
}


///
/// \brief rdb::increase
/// \param i
///
/// \details Добавление вокселя к указанной стороне
///
void area::increase(int)
{
/*
  if(id > (render_indices/indices_per_side) * bytes_per_side) return;

  GLsizeiptr offset = (id/vertices_per_side) * bytes_per_side;
  auto pVox = mVBO[offset];                  // Указатель на выделенный воксель
  u_char s0 = pVox->side_id_by_offset(offset); // Направление стороны
  i3d P0 = pVox->Origin;                       // координаты опорной точки
  P0 = i3d_near(P0, s0);                      // точка размещения рядом нового элемента

  for (u_char side = 0; side < SIDES_COUNT; ++side) // Временно убрать из рендера воксели
    voxel_wipe(get(i3d_near(P0, side)));           // вокруг изменяемого/нового вокселя

  // Нарисовать в опорной точке воксель. Автоматически производится пересчет видимости
  // сторон соседних вокселей вокруг только что созданного.
  voxel_draw(add_voxel(P0));

  for (u_char side = 0; side < SIDES_COUNT; ++side) // Вернуть в рендер соседние воксели
    voxel_draw(get(i3d_near(P0, side)));
    */
}


///
/// \brief rdb::decrease
/// \param i - порядковый номер группы данных из буфера
///
/// \details Удаление вокселя
///
void area::decrease(int)
{
/*  if(i > (render_indices/indices_per_side) * bytes_per_side) return;

  // по номеру группы данных вычисляем ее адрес смещения в VBO
  GLsizeiptr offset = (i/vertices_per_side) * bytes_per_side;

  std::unique_ptr<voxel> pVox = mVBO[offset];                       // По адресу смещения найдем воксель,
  i3d P0 = pVox->Origin;                            // по нему - опорную точку. Временно
  for (u_char side = 0; side < SIDES_COUNT; ++side) // убрать из рендера воксели вокруг
    voxel_wipe(get(i3d_near(P0, side)));            // найденой опорной точки

  voxel_wipe(pVox);                                 // Выделенный воксель убрать из рендера,

  // TODO
  //mArea.erase(P0);                                  // удалить из базы данных,

  recalc_around_visibility(P0);                     // пересчитать видимость вокселей вокруг
  for (u_char side = 0; side < SIDES_COUNT; ++side) // Вернуть в рендер соседние воксели
    voxel_draw(get(i3d_near(P0, side)));
*/
}


///
/// \brief rdb::rig_display
///
/// \details Добавление в графический буфер элементов
///
/// Координаты вершин хранятся в нормализованом виде, поэтому перед записью
/// в VBO данные копируются во временный кэш, где координаты пересчитываются
/// с учетом координат вокселя, после чего записываются в VBO. Адрес
/// смещения данных в VBO запоминается.
///
void area::voxel_draw(std::unique_ptr<voxel> pVox)
{
  if(nullptr == pVox) return;   // TODO: тут можно подгружать или дебажить
  if(pVox->in_vbo) return;      // Если данные уже в VBO - ничего не делаем

  GLsizeiptr vbo_addr = 0;
  GLfloat buffer[digits_per_side];

  // каждая сторона вокселя обрабатывается отдельно
  for(u_char side_id = 0; side_id < SIDES_COUNT; ++side_id)
  {
    if(!pVox->side_fill_data(side_id, buffer)) continue;
    vbo_addr = pVBO->append(buffer, bytes_per_side);
    pVox->offset_write(side_id, vbo_addr);          // запомнить в вокселе его адрес смещения VBO
    // добавить адрес вокселя
    mVBO[vbo_addr] = pVox->Origin;
    render_indices += indices_per_side;             // увеличить число точек рендера
  }
  pVox->in_vbo = true;
}


///
/// \brief rdb::remove_from_vbo
/// \param x
/// \param y
/// \param z
/// \details Стереть риг в рендере
///
void area::voxel_wipe(std::unique_ptr<voxel> pVox)
{
  if(nullptr == pVox) return;
  if(!pVox->in_vbo) return;
  pVox->in_vbo = false;

  for(u_char side_id = 0; side_id < SIDES_COUNT; ++side_id)
  {
    GLsizeiptr dest = pVox->offset_read(side_id);  // адрес данных, которые будут перезаписаны
    if(dest < 0) continue;                         // -1 если сторона невидима - пропустить цикл
    pVox->offset_write(side_id, -1);
    GLsizeiptr free = pVBO->remove(dest, bytes_per_side); // убрать снип из VBO и сохранить его адрес смещения
                                                         // в VBO с которого данные были перенесены на dest
    if(free == 0) // Если VBO пустой
    {
      mVBO.clear();
      render_indices = 0;
      return;
    }

    if (free != dest)                     // Если удаляемый блок данных не в конце, то на его
    {                                     // место переписываются данные с адреса free.

      //!!!TODO!!!
      //!
      //! Операции ниже использовались для вокселей, данные которых хранились в кэше в ОЗУ.
      //! Теперь данные вокселей находятся в базе данных - следует пересмотреть это место.
      //!
      //auto pVoxMov = cfg::DataBase.get_voxel(mVBO[free], voxel_size); // Адрес вокселя, чьи данные переносятся.
      //pVoxMov->offset_replace(free, dest);// изменить хранимый в боксе адрес размещения данных в VBO;

      mVBO[dest] = mVBO[free];               // заменить адрес блока в карте.
    }
                                          // Если free == dest, то только удалить адрес с карты
    mVBO.erase(free);                     // удалить освободившийся элемент массива;
    render_indices -= indices_per_side;   // Уменьшить число точек рендера
  }
}


///
/// \brief space::init
///
void area::init(vbo_ext* v)
{
  pVBO = v; // привязка VBO
  pVBO->clear();

  mVBO.clear();
  render_indices = 0;

  // Origin вокселя, в котором расположена камера
  Location = { static_cast<int>(floor(Eye.ViewFrom.x / voxel_size)) * voxel_size,
                static_cast<int>(floor(Eye.ViewFrom.y / voxel_size)) * voxel_size,
                static_cast<int>(floor(Eye.ViewFrom.z / voxel_size)) * voxel_size };
  MoveFrom = Location;

  int lod_side = 8 * voxel_size; // размер стороны LOD области для данных, запрашиваемых из БД
  for(int x = 0; x<= lod_side; x += voxel_size)
    for(int y = 0; y<= lod_side; y += voxel_size)
      for(int z = 0; z<= lod_side; z += voxel_size)
        voxel_draw(cfg::DataBase.get_voxel({x, y, z}, voxel_size));
}


///
/// \brief space::redraw_borders_x
/// \details Построение границы области по оси X по ходу движения
///
void area::redraw_borders_x(void)
{
  int yMin = -area_width;              // Y-граница LOD
  int yMax =  area_width;

  int x_show, x_hide;
  if(Location.x > MoveFrom.x)
  {        // Если направление движение по оси Х
    x_show = Location.x + area_width; // X-линия вставки вокселей на границе LOD
    x_hide = MoveFrom.x - area_width; // X-линия удаления вокселей на границе
  } else { // Если направление движение против оси Х
    x_show = Location.x - area_width; // X-линия вставки вокселей на границе LOD
    x_hide = MoveFrom.x + area_width; // X-линия удаления вокселей на границе
  }

  int zMin, zMax;

  // Скрыть элементы с задней границы области
  zMin = MoveFrom.z - area_width;
  zMax = MoveFrom.z + area_width;
  for(int y = yMin; y <= yMax; y += voxel_size)
    for(int z = zMin; z <= zMax; z += voxel_size)
      voxel_wipe(cfg::DataBase.get_voxel({x_hide, y, z}, voxel_size));

  // Добавить линию элементов по направлению движения
  zMin = Location.z - area_width;
  zMax = Location.z + area_width;
  for(int y = yMin; y <= yMax; y += voxel_size)
    for(int z = zMin; z <= zMax; z += voxel_size)
      voxel_draw(cfg::DataBase.get_voxel({x_show, y, z}, voxel_size));

  MoveFrom.x = Location.x;
}


///
/// \brief space::redraw_borders_z
/// \details Построение границы области по оси Z по ходу движения
///
void area::redraw_borders_z(void)
{
  int yMin = -area_width;              // Y-граница LOD
  int yMax =  area_width;

  int z_show, z_hide;
  if(Location.z > MoveFrom.z)
  {        // Если направление движение по оси Z
    z_show = Location.z + area_width; // Z-линия вставки вокселей на границе LOD
    z_hide = MoveFrom.z - area_width; // Z-линия удаления вокселей на границе
  } else { // Если направление движение против оси Z
    z_show = Location.z - area_width; // Z-линия вставки вокселей на границе LOD
    z_hide = MoveFrom.z + area_width; // Z-линия удаления вокселей на границе
  }

  int xMin, xMax;

  // Скрыть элементы с задней границы области
  xMin = MoveFrom.x - area_width;
  xMax = MoveFrom.x + area_width;
  for(int y = yMin; y <= yMax; y += voxel_size)
    for(int x = xMin; x <= xMax; x += voxel_size)
      voxel_wipe(cfg::DataBase.get_voxel({x, y, z_hide}, voxel_size));

  // Добавить линию элементов по направлению движения
  xMin = Location.x - area_width;
  xMax = Location.x + area_width;
  for(int y = yMin; y <= yMax; y += voxel_size)
    for(int x = xMin; x <= xMax; x += voxel_size)
      voxel_draw(cfg::DataBase.get_voxel({x, y, z_show}, voxel_size));

  MoveFrom.z = Location.z;
}


///
/// \brief space::recalc_borders
/// \details Перестроение границ активной области при перемещении камеры
///
/// TODO? (на случай притормаживания - если прыгать камерой туда-сюда через
/// границу запуска перерисовки границ) можно процедуры "redraw_borders_?"
/// разбить по две части - вперел/назад.
///
void area::recalc_borders(void)
{
  // Origin вокселя, в котором расположена камера
  Location = { static_cast<int>(floor(Eye.ViewFrom.x / voxel_size)) * voxel_size,
                static_cast<int>(floor(Eye.ViewFrom.y / voxel_size)) * voxel_size,
                static_cast<int>(floor(Eye.ViewFrom.z / voxel_size)) * voxel_size };

  if(Location.x != MoveFrom.x) redraw_borders_x();
  if(Location.z != MoveFrom.z) redraw_borders_z();
}


///
/// \brief rdb::gen_rig
/// \param P
///
/// \details В указанной 3D точке генерирует воксель.
///
std::unique_ptr<voxel> area::add_voxel(const i3d& P)
{
  auto pVox = cfg::DataBase.get_voxel(P, voxel_size);                   // Если в этой точке уже есть воксель,
  if(nullptr != pVox) return pVox;      // то возвращается ссылка на него;

  // TODO: добавить в базу данных новый воксель
  //mArea.emplace(P, std::pair<const i3d&, int>(P, voxel_size));

  pVox = std::make_unique<voxel> (P, voxel_size);
  recalc_voxel_visibility(pVox.get());
  return pVox;
}


///
/// \brief rdb::visibility_recalc
/// \param R0
/// \details Пересчет видимости сторон вокса и его соседей вокруг него
///
void area::recalc_voxel_visibility(voxel* pVox)
{
  if(nullptr == pVox) return;
  std::unique_ptr<voxel> pVoxNear = nullptr;

  for (u_char side = 0; side < SIDES_COUNT; ++side)
  { // С каждой стороны вокселя проверить наличие соседнего вокселя
    pVoxNear = cfg::DataBase.get_voxel(i3d_near(pVox->Origin, side), voxel_size);
    if(nullptr != pVoxNear)                           // Если есть соседний,
    {                                                 // то соприкасающиеся
      pVox->visible[side] = false;                    // стороны невидимы
      pVoxNear->visible[side_opposite(side)] = false; // у обоих вокселей.
    }
    else { pVox->visible[side] = true; }              // Иначе - сторона видимая.
  }
}


///
/// \brief rdb::visibility_recalc
/// \param P0
/// \details Пересчет видимости вокселей вокруг опорной точки
///
void area::recalc_around_visibility(i3d P0)
{
  auto pVox = cfg::DataBase.get_voxel(P0, voxel_size);
  if(nullptr != pVox)
  {                                 // Если в этой точке есть воксель,
    recalc_voxel_visibility(pVox.get()); // то вызвать пересчет видимости
    return;                         // его сторон и выйти.
  }

  // Если в указанной точке пусто, то у вокселей вокруг нее
  // включить видимость прилегающих сторон
  for (u_char side = 0; side < SIDES_COUNT; ++side)
  {
    pVox = cfg::DataBase.get_voxel(i3d_near(P0, side), voxel_size);
    if(nullptr != pVox) pVox->visible[side_opposite(side)] = true;
  }
}

} //namespace
