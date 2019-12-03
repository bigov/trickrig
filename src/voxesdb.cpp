#include "voxesdb.hpp"

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
/// \brief vox_buffer::i3d_near
/// \param P
/// \param s
/// \param l
/// \return
///
///  Координаты опорной точки соседнего вокселя относительно указанной стороны
///
i3d i3d_near(const i3d& P, u_char side, int side_len)
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
/// \brief voxesdb::append
/// \param offset
/// \details Добавление вокса к строне существующего вокса
/// через указание адреса размещения этой стороны в VBO
///
void voxesdb::append(GLsizeiptr offset)
{
  std::lock_guard<std::mutex> Hasp{mutex_voxes_db};

  vox* pVox = get(offset);
  if (nullptr == pVox) return;
  auto vox_side_len = pVox->side_len;
  i3d P0 = i3d_near(pVox->Origin, pVox->side_id_by_offset(offset), vox_side_len); // координаты нового вокса

  for (u_char side = 0; side < SIDES_COUNT; ++side) // Временно убрать из рендера воксы
    remove_from_vbo(get(i3d_near(P0, side, vox_side_len)));   // вокруг точки добавления вокса

  // Добавить вокс в БД и в рендер VBO
  vox* V = create(P0, vox_side_len);
  recalc_vox_visibility(V);
  append_in_vbo(V);

  for (u_char side = 0; side < SIDES_COUNT; ++side) // Вернуть в рендер соседние воксы
    append_in_vbo(get(i3d_near(P0, side, vox_side_len)));
}


///
/// \brief voxesdb::remove
/// \param offset
/// \details Удаление вокса по адресу размещения стороны в VBO
///
void voxesdb::remove(GLsizeiptr offset)
{
  std::lock_guard<std::mutex> Hasp{mutex_voxes_db};

  vox* V = get(offset);
  if(nullptr == V) ERR("Error: try to remove unexisted vox");
  auto vox_side_len = V->side_len;
  i3d P0 = V->Origin;

  for (u_char side = 0; side < SIDES_COUNT; ++side) // Временно убрать из рендера воксы вокруг
    remove_from_vbo(get(i3d_near(P0, side, vox_side_len)));

  remove_from_vbo(V);                          // Выделенный вокс убрать из рендера
  cfg::DataBase.erase_vox(V);                  // удалить из базы данных,

  auto it = std::find_if(begin(), end(), [&P0](const auto& V){ return V->Origin == P0; });
  if(it != end()) erase(it);

  recalc_around_visibility(P0, vox_side_len);  // пересчитать видимость вокселей вокруг

  for (u_char side = 0; side < SIDES_COUNT; ++side) // Вернуть в рендер соседние воксели
    append_in_vbo(get(i3d_near(P0, side, vox_side_len)));
}


///
/// \brief voxesdb::vox_add_in_db
/// \param P
/// \param side_len
/// \return
/// \details В указанной 3D точке генерирует новый вокс c пересчетом
/// видимости своих сторон и окружающих воксов и вносит его в базу данных
///
vox* voxesdb::create(const i3d& P, int side_len)
{
  auto pVox = cfg::DataBase.get_vox(P);
  if (nullptr == pVox)
  {
    pVox = std::make_unique<vox> (P, side_len);
    cfg::DataBase.save_vox(pVox.get());
  }
  return push_db(std::move(pVox));
}


///
/// \brief voxesdb::push_back
/// \param V
/// \return
///
vox* voxesdb::push_db(std::unique_ptr<vox> V)
{
  push_back(std::move(V));
  return back().get();
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
/// \param R0
/// \details Пересчет видимости сторон вокса и его соседей вокруг него
///
void voxesdb::recalc_vox_visibility(vox* pVox)
{
  if (nullptr == pVox) return;

  // С каждой стороны вокса проверить наличие соседа
  for (u_char side = 0; side < SIDES_COUNT; ++side)
  {
    vox* pNear = get(i3d_near(pVox->Origin, side, pVox->side_len));
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
  for (u_char side = 0; side < SIDES_COUNT; ++side)
  {
    pVox = get(i3d_near(P0, side, side_len));
    if (nullptr != pVox) pVox->visible_on(side_opposite(side));
  }
}


///
/// \brief voxesdb::vox_draw
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
/// \brief voxesdb::vox_wipe
/// \param vox* pVox
/// \details Убрать вокс из рендера
///
void voxesdb::remove_from_vbo(vox* pVox)
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
      vox* V = get(free); // Найти вокс, чьи данные были перенесены
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
/// \brief voxesdb::vox_load
/// \param P0
/// \details Загрузить вокс из базы данных в буфер и в рендер
///
void voxesdb::load(const i3d& P0)
{
  std::lock_guard<std::mutex> Hasp{mutex_voxes_db};

  auto pVox = cfg::DataBase.get_vox(P0);

  if(nullptr != pVox)
  {
    vox* V = push_db(std::move(pVox));

    for (u_char side = 0; side < SIDES_COUNT; ++side) // Временно убрать из рендера воксы
      remove_from_vbo(get(i3d_near(P0, side, V->side_len)));   // вокруг точки добавления вокса

    recalc_vox_visibility(V); // пересчитать видимость сторон
    append_in_vbo(V);         // добавить в рендер

    for (u_char side = 0; side < SIDES_COUNT; ++side) // Вернуть в рендер соседние воксы
      append_in_vbo(get(i3d_near(P0, side, V->side_len)));
  }
}


///
/// \brief voxesdb::vox_unload
/// \param P0
/// \details Выгрузить данные вокса из памяти, и убрать из рендера
///
void voxesdb::unload(const i3d& P0)
{
  std::lock_guard<std::mutex> Hasp{mutex_voxes_db};

  auto it = std::find_if(begin(), end(), [&P0](const auto& V){ return V->Origin == P0; });
  if(it == end()) return; // в этой точке вокса нет
  vox* V = it->get();
  if(V->in_vbo) remove_from_vbo(V);
  erase(it);
}

} // namespace tr

