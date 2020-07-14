#include "voxesdb.hpp"

namespace tr
{

///
/// \brief cross
/// \param s
/// \return номер стороны, противоположной указанной в параметре
///
uchar side_opposite(uchar s)
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
i3d i3d_near(const i3d& P, uchar side, int side_len)
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
voxesdb::voxesdb (vbo *V): pVBO(V)
{

}


///
/// \brief voxesdb::vox_create
/// \param P0
/// \param vox_side_len
///
//void voxesdb::vox_create(const i3d& P0, int vox_side_len)
void voxesdb::vox_create(const i3d&, int)
{
/*
  for (uchar side = 0; side < SIDES_COUNT; ++side)        // Временно убрать из рендера воксы
    remove_from_vbo(get(i3d_near(P0, side, vox_side_len))); // вокруг точки добавления вокса

  auto upVox = std::make_unique<vox> (P0, vox_side_len);    // Новый вокс
  //cfg::DataBase.save_vox(upVox.get());                      // Записать в БД
  push_back(std::move(upVox));                              // Записать в буфер
  vox* V = back().get();
  recalc_vox_visibility(V);                                 // пересчитать видимость сторон
  append_in_vbo(V);                                         // добавить в VBO на рендер

  for (uchar side = 0; side < SIDES_COUNT; ++side) // Вернуть в рендер соседние воксы
    append_in_vbo(get(i3d_near(P0, side, vox_side_len)));
*/
}


///
/// \brief voxesdb::remove
/// \param offset
/// \details Удаление вокса по адресу размещения стороны в VBO
///
void voxesdb::remove(GLsizeiptr)
//void voxesdb::remove(GLsizeiptr offset)
{
  /*

  vox* V = get(offset);
  if(nullptr == V) ERR("Error: try to remove unexisted vox");
  auto vox_side_len = V->side_len;
  i3d P0 = V->Origin;

  for (uchar side = 0; side < SIDES_COUNT; ++side) // Временно убрать из рендера воксы вокруг
    remove_from_vbo(get(i3d_near(P0, side, vox_side_len)));

  remove_from_vbo(V);                          // Выделенный вокс убрать из рендера
  cfg::DataBase.erase_vox(V);                  // удалить из базы данных,
  cfg::DataBase.vox_data_delete(V->Origin.x, V->Origin.y, V->Origin.z);

  auto it = std::find_if(begin(), end(), [&P0](const auto& V){ return V->Origin == P0; });
  if(it != end()) erase(it);

  recalc_around_visibility(P0, vox_side_len);  // пересчитать видимость вокселей вокруг

  for (uchar side = 0; side < SIDES_COUNT; ++side) // Вернуть в рендер соседние воксели
    append_in_vbo(get(i3d_near(P0, side, vox_side_len)));
*/
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
  for (uchar side = 0; side < SIDES_COUNT; ++side)
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
  for (uchar side = 0; side < SIDES_COUNT; ++side)
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

} // namespace tr

