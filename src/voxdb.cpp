/*
 *
 * file: rdb.cpp
 *
 * Управление элементами 3D пространства
 *
 */

#include "voxdb.hpp"

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
/// \brief rdb::caps_lock_toggle
/// \details переключить положение caps_lock
///
void voxdb::caps_lock_toggle(void)
{
  caps_lock = !caps_lock;
}


///
/// \brief rdb::increase
/// \param i
///
/// \details Добавление объема к указанной стороне
///
void voxdb::increase(int id)
{
  if(id > (render_indices/indices_per_quad) * bytes_per_snip) return;

  GLsizeiptr offset = (id/vertices_per_quad) * bytes_per_snip;
  voxel* pVox = Visible[offset];                 // По адресу смещения найдем бокс;
  u_char s0 = pVox->side_id_by_offset(offset); // Направление стороны бокса;
  i3d P0 = pVox->Origin;                       // по ригу - опорную точку;
  P0 = i3d_shift(P0, s0, lod);              // точка генерации нового рига рядом

  for (u_char side = 0; side < SIDES_COUNT; ++side) // Временно убрать из рендера риги
    voxel_wipe(get(i3d_shift(P0, side, lod)));        // вокруг изменяемого (или нового) рига

  // Нарисовать в опорной точке риг. Если сторона была полная, то создается новый риг и
  // автоматически производится пересчет видимости сторон соседних ригов вокруг только
  // что созданного. Если риг уже существует, то "gen_rig" выдает ссылку на него и он рисуется.
  voxel_draw(add_voxel_on_map(P0));

  for (u_char side = 0; side < SIDES_COUNT; ++side) // Вернуть в рендер соседние риги
    voxel_draw(get(i3d_shift(P0, side, lod)));
}


///
/// \brief rdb::decrease
/// \param i - порядковый номер группы данных из буфера
///
/// \details Уменьшение объема с указанной стороны
///
void voxdb::decrease(int i)
{
  if(i > (render_indices/indices_per_quad) * bytes_per_snip) return;

  // по номеру группы данных вычисляем ее адрес смещения в VBO
  GLsizeiptr offset = (i/vertices_per_quad) * bytes_per_snip;

  voxel* pVox = Visible[offset];                 // По адресу смещения найдем бокс

  i3d P0 = pVox->Origin;                       // по ригу - опорную точку
  //u_char s0 = B->side_id_by_offset(offset); // Направление стороны бокса;

  for (u_char side = 0; side < SIDES_COUNT; ++side) // Убрать из рендера риги вокруг
    voxel_wipe(get(i3d_shift(P0, side, lod)));

  voxel_wipe(pVox);                               // убрать из рендера выбранный риг
  VoxMap.erase(pVox->Origin);                  // удалить риг из базы данных
  visibility_recalc(P0);                     // пересчитать видимость ригов вокруг
  for (u_char side = 0; side < SIDES_COUNT; ++side) // Вернуть в рендер соседние риги
    voxel_draw(get(i3d_shift(P0, side, lod)));
}


///
/// \brief rdb::rig_display
///
/// \details Добавление в графический буфер элементов рига
///
/// Координаты вершин хранятся в нормализованом виде, поэтому перед записью
/// в VBO данные копируются во временный кэш, где координаты пересчитываются
/// с учетом координат рига (TODO: сдвига и поворота рига-контейнера),
/// после чего записываются в VBO. Адрес смещения данных в VBO запоминается.
///
void voxdb::voxel_draw(voxel* pVox)
{
  if(nullptr == pVox) return;   // TODO: тут можно подгружать или дебажить
  if(pVox->in_vbo) return;      // Если данные уже в VBO - ничего не делаем

  GLsizeiptr vbo_addr = 0;
  GLfloat buffer[digits_per_snip];

  for(u_char side_id = 0; side_id < SIDES_COUNT; ++side_id)
  {
    if(!pVox->side_fill_data(side_id, buffer, pVox->Origin)) continue;
    vbo_addr = VBO->append(buffer, bytes_per_snip);
    pVox->offset_write(side_id, vbo_addr);             // записать в бокс адрес смещения VBO
    Visible[vbo_addr] = pVox;                        // добавить ссылку на бокс
    render_indices += indices_per_quad;             // увеличить число точек рендера
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
void voxdb::voxel_wipe(voxel* pVox)
{
  if(nullptr == pVox) return;
  if(!pVox->in_vbo) return;
  pVox->in_vbo = false;

  for(u_char side_id = 0; side_id < SIDES_COUNT; ++side_id)
  {
    GLsizeiptr dest = pVox->offset_read(side_id);  // адрес данных, которые будут перезаписаны
    if(dest < 0) continue;                     // -1 если сторона невидима - пропустить цикл
    pVox->offset_write(side_id, -1);
    GLsizeiptr free = VBO->remove(dest, bytes_per_snip); // убрать снип из VBO и сохранить его адрес смещения
                                                         // в VBO с которого данные были перенесены на dest
    if(free == 0) // Если VBO пустой
    {
      Visible.clear();
      render_indices = 0;
      return;
    }

    if (free != dest)                      // Если удаляемый блок данных не в конце, то на его
    {                                      // место переписываются данные с адреса free.
      voxel* pVoxMov = Visible[free];      // Адрес вокселя, чьи данные переносятся.
      pVoxMov->offset_replace(free, dest); // изменить хранимый в боксе адрес размещения данных в VBO;
      Visible[dest] = pVoxMov;             // заменить адрес блока в карте снипов.
    }
                                           // Если free == dest, то только удалить адрес с карты
    Visible.erase(free);                   // удалить освободившийся элемент массива;
    render_indices -= indices_per_quad;    // Уменьшить число точек рендера
  }
}


///
/// \brief Загрузка из базы данных в оперативную память блока пространства
///
/// \details  Формирование в оперативной памяти карты ригов (std::map) для
/// выбраной области пространства. Из этой карты берутся данные снипов,
/// размещаемых в VBO для рендера сцены.
///
void voxdb::load_space(vbo_ext* vbo, int l_o_d, const glm::vec3& Position)
{
  VBO = vbo;
  i3d P{ Position };
  lod = l_o_d; // TODO проверка масштаба на допустимость

  // Загрузка фрагмента карты 8х8х(16x16) раз на xz плоскости
  i3d From {P.x - 64, 0, P.z - 64};
  i3d To {P.x + 64, 1, P.z + 64};

  VBO->clear();
  VoxMap.clear();
  Visible.clear();
  render_indices = 0;

  //Загрузка из базы данных
  //cfg::DataBase.rigs_loader(MapRigs, From, To);
  //std::srand(std::time(nullptr));

  int side_length = 20;
  for (int x = -side_length; x < side_length+1; ++x)
    for (int z = -side_length; z < side_length+1; ++z)
  {
    add_voxel_on_map({x,-1, z});
    add_voxel_on_map({x, 0, z});
    add_voxel_on_map({x, 1, z});
  }
}


///
/// \brief rdb::gen_rig
/// \param P
///
/// \details В указанной точке генерирует однобоксовый риг. Можно указать
/// размеры сторон бокса. По-умолчанию они устанавливаются = 255
///
voxel* voxdb::add_voxel_on_map(const i3d& P)
{
  voxel* pVox = get(P);             // Если в этой точке уже есть воксель,
  if(nullptr != pVox) return pVox;  // то возвращается ссылка на него.
                                    // Иначе - создаетя новый риг
  VoxMap.emplace(P, P);
  pVox = get(P);
  visibility_voxel_recalc(pVox);
  return pVox;
}


///
/// \brief rdb::visibility_recalc
/// \param R0
/// \details Пересчет видимости сторон вокса и его соседей вокруг него
///
void voxdb::visibility_voxel_recalc(voxel* pVox0)
{
  if(nullptr == pVox0) return;

  voxel* pVox1 = nullptr;
  for (u_char side = 0; side < SIDES_COUNT; ++side)   // для их каждой стороны,
  {                                                   // получить
    pVox1 = get(i3d_shift(pVox0->Origin, side, lod)); // ссылку на соседний риг.
    if(nullptr == pVox1) continue;                    // Если пусто - пропустить.
    pVox0->visible_check(side, pVox1);                // Проверить стыковку (видимость) стороны
  }                                                   // с каждым из боксов соседнего рига
}


///
/// \brief rdb::visibility_recalc
/// \param P0
/// \details Пересчет видимости ригов вокруг опорной точки
///
void voxdb::visibility_recalc(i3d P0)
{
  voxel* pVox = get(P0);
  if(nullptr != pVox)
  {                               // Если в этой точке есть риг,
    visibility_voxel_recalc(pVox);  // то вызвать пересчет сторон рига
    return;                       // и выйти.
  }

  // Если в указанной точке нет рига, то у ригов вокруг нее
  // следует включить видимость противоположной стороны
  for (u_char side = 0; side < SIDES_COUNT; ++side)
  {
    pVox = get(i3d_shift(P0, side, lod));
    if(nullptr == pVox) continue;
    pVox->visible[opposite(side)] = true;
  }
}


///
/// \brief rdb::get
/// \param P
/// \return
/// \details  Поиск элемента с указанными координатами
///
voxel* voxdb::get(const i3d &P)
{
  if(P.y < yMin)
  {
    ERR("rigs::get -Y is overflow");
  }
  if(P.y > yMax) ERR("rigs::get +Y is overflow");

  try { return &VoxMap.at(P); }
  catch (...) { return nullptr; }
}

} //namespace
