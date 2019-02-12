/*
 *
 * file: rdb.cpp
 *
 * Управление элементами 3D пространства
 *
 */

#include "rdb.hpp"

namespace tr
{

i3d i3d_shift(const i3d& P, u_char s, int l)
{
  switch (s) {
    case SIDE_XP:
      return i3d{ P.x + l, P.y    , P.z     };
      break;
    case SIDE_XN:
      return i3d{ P.x - l, P.y    , P.z     };
      break;
    case SIDE_YP:
      return i3d{ P.x    , P.y + l, P.z     };
      break;
    case SIDE_YN:
      return i3d{ P.x    , P.y - l, P.z     };
      break;
    case SIDE_ZP:
      return i3d{ P.x    , P.y    , P.z + l };
      break;
    case SIDE_ZN:
      return i3d{ P.x    , P.y    , P.z - l };
      break;
    default:
      return P;
  }
}


///
/// \brief rdb::increase
/// \param i
///
/// \details Добавление объема к указанной стороне
///
void rdb::increase(unsigned int i)
{
  if(i > (render_points/indices_per_snip) * bytes_per_snip) return;

  GLsizeiptr offset = (i/vertices_per_snip) * bytes_per_snip;
  box* B = Visible[offset];                 // По адресу смещения найдем бокс;
  u_char s0 = B->side_id_by_offset(offset); // Направление стороны бокса;
  rig* Rig = B->ParentRig;                  // по боксу - риг;
  i3d P0 = Rig->Origin;                     // по ригу - опорную точку;
  P0 = i3d_shift(P0, s0, lod);              // Точка генерации нового рига.

  for (u_char side = 0; side < SIDES_COUNT; ++side) // Временно убрать из рендера риги
    rig_wipeoff(get(i3d_shift(P0, side, lod)));     // вокруг точки создания нового

  // Нарисовать в опорной точке новый риг (автоматически производится пересчет
  // видимости сторон соседних ригов вокруг только что созданного)
  rig_display(gen_rig(P0));

  for (u_char side = 0; side < SIDES_COUNT; ++side) // Вернуть в рендер соседние риги
    rig_display(get(i3d_shift(P0, side, lod)));
}


///
/// \brief rdb::decrease
/// \param i - порядковый номер группы данных из буфера
///
/// \details Уменьшение объема с указанной стороны
///
void rdb::decrease(unsigned int i)
{
  if(i > (render_points/indices_per_snip) * bytes_per_snip) return;

  // При условии, что смещение данных в VBO начинается с нуля, по полученному
  // через параметр номеру группы данных вычисляем адрес ее смещения в буфере
  GLsizeiptr offset = (i/vertices_per_snip) * bytes_per_snip;

  box* B = Visible[offset];     // По адресу смещения найдем бокс
  rig* R = B->ParentRig;        // по боксу - риг
  i3d P0 = R->Origin;           // по ригу - опорную точку

  for (u_char side = 0; side < SIDES_COUNT; ++side) // Временно убрать из рендера риги
    rig_wipeoff(get(i3d_shift(P0, side, lod)));     // вокруг точки удаления

  rig_wipeoff(R);                                   // убрать риг из VBO рендера
  MapRigs.erase(R->Origin);                         // удалить риг из базы данных
  visibility_recalc(P0);                            // пересчитать видимость ригов вокруг

  for (u_char side = 0; side < SIDES_COUNT; ++side) // Вернуть в рендер соседние риги
    rig_display(get(i3d_shift(P0, side, lod)));
}


///
/// Добавление в графический буфер элементов рига
///
void rdb::rig_display(rig* R)
{
  if(nullptr == R) return;   // TODO: тут можно подгружать или дебажить
  if(R->in_vbo) return;      // Если данные уже в VBO - ничего не делаем

  for(box& B: R->Boxes) box_display(B, R->vector());
  R->in_vbo = true;
}


///
/// \brief rdb::box_display
/// \param Box
/// \param Point
///
/// \brief
/// Добавляет данные в VBO
///
/// \details
/// Координаты вершин хранятся в нормализованом виде, поэтому перед записью
/// в VBO данные копируются во временный кэш, где координаты пересчитываются
/// с учетом координат рига (TODO: сдвига и поворота рига-контейнера),
/// после чего записываются в VBO. Адрес смещения данных в VBO запоминается.
///
void rdb::box_display(box& B, const f3d& P)
{
  for (u_char side_id = 0; side_id < SIDES_COUNT; ++side_id)
  {
    side_display(B, side_id, P);
  }
}


///
/// \brief rdb::side_display
/// \param B
/// \param side_id
/// \param P
/// \details Размещение данных стороны в VBO для рендера
///
void rdb::side_display(box& B, u_char side_id, const f3d& P)
{
  GLfloat buffer[digits_per_snip];
  if(!B.side_fill_data(side_id, buffer, P)) return;
  auto offset = VBO->data_append(buffer, bytes_per_snip); // записать в VBO
  render_points += indices_per_snip;                      // увеличить число точек рендера
  B.offset_write(side_id, offset);                        // записать в бокс адрес смещения VBO
  Visible[offset] = &B;                                   // добавить ссылку на бокс
}


///
/// \brief rdb::remove_from_vbo
/// \param x
/// \param y
/// \param z
/// \details Стереть риг в рендере
///
void rdb::rig_wipeoff(rig* Rig)
{
  if(nullptr == Rig) return;
  if(!Rig->in_vbo) return;
  for(box& B: Rig->Boxes) box_wipeoff(B);
  Rig->in_vbo = false;
}


///
/// \brief side_wipeoff
/// \param Side
///
/// \details Удаление из VBO данных указанной стороны
///
void rdb::box_wipeoff(box& Box)
{
  GLsizeiptr target = 0;         // адрес смещения, где данные будут перезаписаны
  GLsizeiptr moving = 0;         // адрес снипа с "хвоста" VBO, который будет перемещен на target

  for(u_char side_id = 0; side_id < SIDES_COUNT; ++side_id)
  {
    if(!Box.visible[side_id]) continue;
    target = Box.offset_read(side_id);            // адрес снипа, подлежащий удалению
    if(target < 0) continue;
    moving = VBO->remove(target, bytes_per_snip); // убрать снип из VBO и получить адрес смещения
                                                  // данных в VBO который изменился на target

    if(moving == 0)                               // Если VBO пустой
    {
      Visible.clear();
      render_points = 0;
      return;
    }
    else if (moving == target)                // Если удаляемый снип оказался в конце активной
    {                                         // части VBO, то только удалить его с карты
      Visible.erase(target);
    }
    else                                      // Если удаляемый блок данных не в конце, то на его
    {                                         // место записаны данные с адреса moving:
      box* B = Visible[moving];
      B->offset_replace(moving, target);      // - изменить адрес размещения в VBO у перемещенного блока,
      Visible[target] = B;                    // - заменить блок в карте снипов,
      Visible.erase(moving);                  // - удалить освободившийся элемент массива
    }
    render_points -= indices_per_snip;        // Уменьшить число точек рендера
  }
}


///
/// \brief Загрузка из базы данных в оперативную память блока пространства
///
/// \details  Формирование в оперативной памяти карты ригов (std::map) для
/// выбраной области пространства. Из этой карты берутся данные снипов,
/// размещаемых в VBO для рендера сцены.
///
void rdb::load_space(vbo_ext* vbo, int l_o_d, const glm::vec3& Position)
{
  VBO = vbo;
  i3d P{ Position };
  lod = l_o_d; // TODO проверка масштаба на допустимость

  // Загрузка фрагмента карты 8х8х(16x16) раз на xz плоскости
  i3d From {P.x - 64, 0, P.z - 64};
  i3d To {P.x + 64, 1, P.z + 64};

  VBO->clear();
  MapRigs.clear();
  Visible.clear();
  render_points = 0;

  // Загрузка из базы данных
  //cfg::DataBase.rigs_loader(MapRigs, From, To);

  for (int x = -4; x < 4; ++x)
    for (int z = -4; z < 4; ++z)
  {
    gen_rig({x,-1, z});
    gen_rig({x, 0, z});
    gen_rig({x, 1, z});
  }
}


///
/// \brief rdb::gen_rig
/// \param P
///
rig* rdb::gen_rig(const i3d& P)
{
  MapRigs[P] = rig{P};
  rig* R = get(P);

  R->Boxes.push_back(box{ {0, 0, 0}, {255, 255, 255}, R});
  visibility_recalc_rigs(R);
  return R;
}


///
/// \brief rdb::visibility_recalc
/// \param R0
/// \details Пересчет видимости сторон целевого рига и соседей вокруг него
///
void rdb::visibility_recalc_rigs(rig* R0)
{
  if(nullptr == R0) return;
  if(R0->zoom < 0) return;
  i3d P0 = R0->Origin;
  rig* R1 = nullptr;

  for(box& B0: R0->Boxes)                                // Для всех боксов рига,
  for (u_char side = 0; side < SIDES_COUNT; ++side)      // для их каждой стороны,
  {                                                      // получить
    R1 = get(i3d_shift(P0, side, lod));                  // ссылку на соседний риг.
    if(nullptr == R1) continue;                          // Если пусто - пропустить.
    for(box& B1: R1->Boxes) B0.visible_check(side, B1);  // Проверить стыковку (видимость) стороны
  }                                                      // с каждым из боксов соседнего рига
}


///
/// \brief rdb::visibility_recalc
/// \param P0
/// \details Пересчет видимости ригов вокруг опорной точки
///
void rdb::visibility_recalc(i3d P0)
{
  rig* Rig = get(P0);
  if(nullptr != Rig)
  {                               // Если в этой точке есть риг,
    visibility_recalc_rigs(Rig);  // то вызвать пересчет сторон рига
    return;                       // и выйти.
  }

  // Если в указанной точке нет рига, то у ригов вокруг нее
  // следует включить видимость противоположной стороны
  for (u_char side = 0; side < SIDES_COUNT; ++side)
  {
    Rig = get(i3d_shift(P0, side, lod));
    if(nullptr == Rig) continue;
    for(box& B: Rig->Boxes) B.visible[opposite(side)] = true;
  }
}

///
/// \brief rdb::search_down
/// \param V
/// \return
/// \details Поиск по координатам ближайшего блока снизу
///
i3d rdb::search_down(const glm::vec3& V)
{
  return search_down(V.x, V.y, V.z);
}


///
/// \brief rdb::search_down
/// \param x
/// \param y
/// \param z
/// \return
/// \details Поиск по координатам ближайшего блока снизу
///
i3d rdb::search_down(float x, float y, float z)
{
  return search_down(
        static_cast<double>(x),
        static_cast<double>(y),
        static_cast<double>(z)
  );
}


///
/// \brief rdb::search_down
/// \param x
/// \param y
/// \param z
/// \return
/// \details Поиск по координатам ближайшего блока снизу
///
i3d rdb::search_down(double x, double y, double z)
{
  return search_down(
    static_cast<int>(floor(x)),
    static_cast<int>(floor(y)),
    static_cast<int>(floor(z))
  );
}


///
/// \brief rdb::search_down
/// \param x
/// \param y
/// \param z
/// \return
/// \default Поиск по координатам ближайшего блока снизу
///
i3d rdb::search_down(int x, int y, int z)
{
  if(y < yMin) ERR("Y downflow"); if(y > yMax) ERR("Y overflow");
  while(y > yMin)
  {
    try
    { 
      MapRigs.at(i3d {x, y, z});
      return i3d {x, y, z};
    } catch (...)
    { y -= lod; }
  }
  ERR("Rigs::search_down() failure. We need to use try/catch in this case.");
}


///
/// \brief rdb::get
/// \param P
/// \return
/// \details  Поиск элемента с указанными координатами
///
rig* rdb::get(const i3d &P)
{
  if(P.y < yMin) ERR("rigs::get -Y is overflow");
  if(P.y > yMax) ERR("rigs::get +Y is overflow");

  try { return &MapRigs.at(P); }
  catch (...) { return nullptr; }
}

} //namespace
