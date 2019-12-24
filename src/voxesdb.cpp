#include "voxesdb.hpp"

namespace tr
{

///
/// \brief cross
/// \param s
/// \return номер стороны, противоположной указанной в параметре
///
unsigned char side_opposite(unsigned char s)
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
/// \brief vox_buffer::i3d_near
/// \param P
/// \param s
/// \param l
/// \return
///
///  Координаты опорной точки соседнего вокселя относительно указанной стороны
///
i3d i3d_near(const i3d& P, unsigned char side, int side_len)
{
  switch (side) {
    case SIDE_XP: return i3d{ P.x + side_len, P.y, P.z };
    case SIDE_XN: return i3d{ P.x - side_len, P.y, P.z };
    case SIDE_YP: return i3d{ P.x, P.y + side_len, P.z };
    case SIDE_YN: return i3d{ P.x, P.y - side_len, P.z };
    case SIDE_ZP: return i3d{ P.x, P.y, P.z + side_len };
    case SIDE_ZN: return i3d{ P.x, P.y, P.z - side_len };
    default:      return P;
  }
}


///
/// \brief voxesdb::voxesdb
/// \param V
///
voxesdb::voxesdb (vbo_ext* V): pVBO(V)
{
  // Зарезервировать место в соответствии с размером буфера VBO
  GpuMap.resize(pVBO->max_size()/bytes_per_side);
}


///
/// \brief voxesdb::append
/// \param offset
/// \details Добавление вокса к строне существующего вокса
/// через указание адреса размещения этой стороны в VBO
///
void voxesdb::append(GLsizeiptr offset)
{
  //debug |->
  return;
  //debug ->|

  vox* pVox = get(offset);
  if (nullptr == pVox) return;
  auto vox_side_len = pVox->side_len;
  i3d P0 = i3d_near(pVox->Origin, pVox->side_id_by_offset(offset), vox_side_len); // координаты нового вокса
  vox_create(P0, vox_side_len);
}


///
/// \brief voxesdb::vox_create
/// \param P0
/// \param vox_side_len
///
void voxesdb::vox_create(const i3d&P0, int vox_side_len)
{
  for (unsigned char side = 0; side < SIDES_COUNT; ++side)        // Временно убрать из рендера воксы
    remove_from_vbo(get(i3d_near(P0, side, vox_side_len))); // вокруг точки добавления вокса

  auto upVox = std::make_unique<vox> (P0, vox_side_len);    // Новый вокс
  //cfg::DataBase.save_vox(upVox.get());                      // Записать в БД
  push_back(std::move(upVox));                              // Записать в буфер
  vox* V = back().get();
  recalc_vox_visibility(V);                                 // пересчитать видимость сторон
  append_in_vbo(V);                                         // добавить в VBO на рендер

  for (unsigned char side = 0; side < SIDES_COUNT; ++side) // Вернуть в рендер соседние воксы
    append_in_vbo(get(i3d_near(P0, side, vox_side_len)));
}


///
/// \brief voxesdb::remove
/// \param offset
/// \details Удаление вокса по адресу размещения стороны в VBO
///
void voxesdb::remove(GLsizeiptr offset)
{
  //debug |->
  return;
  //debug ->|

  vox* V = get(offset);
  if(nullptr == V) ERR("Error: try to remove unexisted vox");
  auto vox_side_len = V->side_len;
  i3d P0 = V->Origin;

  for (unsigned char side = 0; side < SIDES_COUNT; ++side) // Временно убрать из рендера воксы вокруг
    remove_from_vbo(get(i3d_near(P0, side, vox_side_len)));

  remove_from_vbo(V);                          // Выделенный вокс убрать из рендера
  cfg::DataBase.erase_vox(V);                  // удалить из базы данных,
  cfg::DataBase.vox_data_delete(V->Origin.x, V->Origin.y, V->Origin.z);

  auto it = std::find_if(begin(), end(), [&P0](const auto& V){ return V->Origin == P0; });
  if(it != end()) erase(it);

  recalc_around_visibility(P0, vox_side_len);  // пересчитать видимость вокселей вокруг

  for (unsigned char side = 0; side < SIDES_COUNT; ++side) // Вернуть в рендер соседние воксели
    append_in_vbo(get(i3d_near(P0, side, vox_side_len)));
}


///
/// \brief voxesdb::vox_by_vbo
/// \param addr
/// \return
/// \details Поиск вокса, у которого сторона размещена в VBO по указанному адресу
vox* voxesdb::get(GLsizeiptr addr)
{
  auto FindResult = std::find_if( begin(), end(),
    [&addr](auto& V)
    {
      if(!V->in_vbo) return false;
      return V->side_id_by_offset(addr) < SIDES_COUNT;
    }
  );
  if (FindResult == end()) return nullptr;
  else return FindResult->get();
}


///
/// \brief voxesdb::vox_by_i3d
/// \param P0
/// \return
/// \details Поиск вокса по его координатам
///
vox* voxesdb::get(const i3d& P0)
{
  auto it = std::find_if(begin(), end(),
                         [&P0](const auto& V){ return V->Origin == P0; });
  if (it == end()) return nullptr;
  else return it->get();
}


///
/// \brief voxesdb::recalc_vox_visibility
/// \param pVox - ссылка на вокс
///
/// \details Пересчет видимости сторон вокса и прилегающих сторон соседей. Если
/// после пересчета видимость сторон изменяется, то информация в БД обновляется.
///
void voxesdb::recalc_vox_visibility(vox* pVox)
{
  if (nullptr == pVox) return;
  auto VoxSides = pVox->get_visibility();

  // С каждой стороны вокса проверить наличие соседа
  for (unsigned char side = 0; side < SIDES_COUNT; ++side)
  {
    vox* pNear = get(i3d_near(pVox->Origin, side, pVox->side_len));
    if (nullptr != pNear)                      // Если есть соседний,
    {                                          // то соприкасающиеся
      auto NearSides = pNear->get_visibility();
      pVox->visible_off(side);                 // стороны невидимы
      pNear->visible_off(side_opposite(side)); // у обоих воксов.
      if(NearSides != pNear->get_visibility()) cfg::DataBase.vox_data_save(pNear);
    } else
    {
      pVox->visible_on(side);                  // Иначе - сторона видимая.
    }
  }
  if (VoxSides != pVox->get_visibility()) cfg::DataBase.vox_data_save(pVox);
}


///
/// \brief voxesdb::recalc_around_visibility
/// \param P0
/// \details Пересчет видимости воксов вокруг опорной точки
///
void voxesdb::recalc_around_visibility(i3d P0, int side_len)
{
  vox* pVox = get(P0);
  if (nullptr != pVox)
  {                              // Если в этой точке есть вокс,
    recalc_vox_visibility(pVox); // то вызвать пересчет видимости
    return;                      // его сторон и выйти.
  }

  // Если в указанной точке пусто, то у воксов вокруг нее
  // надо включить видимость прилегающих сторон
  for (unsigned char side = 0; side < SIDES_COUNT; ++side)
  {
    pVox = get(i3d_near(P0, side, side_len));
    if (nullptr != pVox)
    {
      auto VoxSides = pVox->get_visibility();
      pVox->visible_on(side_opposite(side));
      if(VoxSides != pVox->get_visibility()) cfg::DataBase.vox_data_save(pVox);
    }
  }
}


///
/// \brief voxesdb::append_in_vbo
///
/// \details Добавление в графический буфер элементов
///
/// Координаты вершин хранятся в нормализованом виде, поэтому перед записью
/// в VBO данные копируются во временный кэш, где координаты пересчитываются
/// с учетом координат вокселя, после чего записываются в VBO. Адрес
/// размещения данных в VBO для каждой стороны вокселя сохраняется в параметрах вокселя.
///
void voxesdb::append_in_vbo(vox* pVox)
{
  if(nullptr == pVox) return;
#ifndef NDEBUG
  if(nullptr == pVBO) ERR("Need to init VBO pointer");
  if(pVox->in_vbo) ERR("vox_draw: duplicate draw");
#endif

  GLsizeiptr vbo_addr = 0;
  GLfloat buffer[digits_per_side];

  // каждая сторона вокса обрабатывается отдельно
  for(unsigned char side_id = 0; side_id < SIDES_COUNT; ++side_id)
  {
    if(!pVox->side_fill_data(side_id, buffer)) continue;
    vbo_addr = pVBO->append(buffer, bytes_per_side);
    pVox->offset_write(side_id, vbo_addr);     // запомнить адрес смещения стороны в VBO

    GpuMap[vbo_addr/bytes_per_side] = pVox->Origin;

    render_indices.fetch_add(indices_per_side);        // увеличить число точек рендера
  }
  pVox->in_vbo = true;
}


///
/// \brief voxesdb::vbo_expand
/// \param data  блок данных видимых сторон
/// \param n     количество (видимых) сторон
/// \param P     координаты вокса
///
/// \details     Размещение вокса в VBO. При вызове указывается адрес
/// размещения данных, число сторон в блоке и 3D координаты вокса
///
void voxesdb::vbo_expand(unsigned char* data, unsigned char n, const i3d& P)
{
  GLsizeiptr vbo_addr = 0;
  while (n > 0) {
    n--;
    vbo_addr = pVBO->append(data, bytes_per_side); // добавить данные стороны в VBO
    GpuMap[vbo_addr/bytes_per_side] = P;           // запомнить положение блока данных
    render_indices.fetch_add(indices_per_side);    // увеличить число точек рендера
    data += bytes_per_side;                        // переключить на следующую сторону
  }
}


///
/// \brief voxesdb::remove_from_vbo
/// \param vox* pVox
/// \details Убрать вокс из рендера
///
void voxesdb::remove_from_vbo(vox* pVox)
{
  if(nullptr == pVox) return;
  if(!pVox->in_vbo) return;

  // Данные каждой из сторон перезаписываются данными воксов с хвоста VBO
  for(unsigned char side_id = 0; side_id < SIDES_COUNT; ++side_id)
  {
    if(!pVox->is_visible(side_id)) continue;      // если сторона невидима, то пропустить цикл
    GLsizeiptr dest = pVox->offset_read(side_id); // адрес освобождаемого блока данных
    pVox->offset_write(side_id, -1);              // чтобы функция поиска больше не находила этот вокс

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
      vox* V = get(free); // Найти вокс, чьи данные были перенесены
      if(nullptr == V) ERR("vox_wipe: find_in_buffer(" + std::to_string(free) + ")");
      V->offset_replace(free, dest); // скорректировать адрес размещения данных

      GpuMap[dest/bytes_per_side] = GpuMap[free/bytes_per_side];
    }
    // Если free == dest, то удаляемый блок данных был в конце VBO,
    // поэтому только уменьшаем число точек рендера
    render_indices.fetch_sub(indices_per_side);
  }
  pVox->in_vbo = false;
}


///
/// \brief voxesdb::vbo_truncate
/// \param P
/// \details Удаляет из рендера данные воксов с линии (x,z)
///
void voxesdb::vbo_truncate(int x, int z)
{
  if (render_indices < indices_per_side) return;
  GLsizeiptr dest, moved_from;
  i3d Pt {0,0,0};

  size_t id = render_indices/indices_per_side;
  while(id > 0)
  {
    id -= 1;
    Pt = GpuMap[id];
    if((Pt.x == x) and (Pt.z == z)) // Данные вокса есть в GPU
    {
      dest = id * bytes_per_side;                      // адрес удаляемого блока данных
      moved_from = pVBO->remove(dest, bytes_per_side); // адрес хвоста VBO (данными отсюда
                                                       // перезаписываются данные по адресу "dest")
      // Если c адреса "free" на "dest" данные были перенесены, то обновить координты Origin
      if (moved_from != dest) GpuMap[dest/bytes_per_side] = GpuMap[moved_from/bytes_per_side];
      // Если free == dest, то удаляемый блок данных был в конце VBO и просто "отбрасывается"
      render_indices.fetch_sub(indices_per_side); // уменьшаем число точек рендера
    }
  }
}


///
/// \brief voxesdb::vox_load
/// \param P0
/// \details Загрузить вокс из базы данных в рендер
///
void voxesdb::load(int x, int z)
{
  // Загрузка из БД
  auto VoxData = cfg::DataBase.load_vox_data(x, z);
  if(VoxData.empty()) return;

  int y = 0;
  size_t offset = 0;
  size_t offset_max = VoxData.size() - sizeof_y - 1;
  while (offset < offset_max)
  {
    memcpy(&y, VoxData.data() + offset, sizeof_y);             // Координата "y"
    offset += sizeof_y;
    std::bitset<6> m(VoxData[offset]);                         // Маcка видимых сторон
    offset += 1;
    vbo_expand(VoxData.data() + offset, m.count(), {x, y, z}); // Занести данные в VBO
    offset += m.count() * bytes_per_side;                      // Перейти к следующему блоку
  }
}

} // namespace tr

