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
/// \brief i3d_shift
/// \param P
/// \param s
/// \param l
/// \return
///
///  Координаты опорной точки соседнего вокселя относительно указанной стороны
///
i3d area::i3d_shift(const i3d& P, u_char side)
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
/// \details Добавление объема к указанной стороне
///
void area::increase(int id)
{
  if(id > (render_indices/indices_per_side) * bytes_per_side) return;

  GLsizeiptr offset = (id/vertices_per_side) * bytes_per_side;
  voxel* pVox = mVBO[offset];                  // Указатель на выделенный воксель
  u_char s0 = pVox->side_id_by_offset(offset); // Направление стороны
  i3d P0 = pVox->Origin;                       // координаты опорной точки
  P0 = i3d_shift(P0, s0);                 // точка размещения рядом нового элемента

  for (u_char side = 0; side < SIDES_COUNT; ++side) // Временно убрать из рендера воксели
    voxel_wipe(get(i3d_shift(P0, side)));      // вокруг изменяемого/нового вокселя

  // Нарисовать в опорной точке воксель. Если сторона была полная, то создается новый, и
  // автоматически производится пересчет видимости сторон соседних вокселей вокруг только
  // что созданного. Если воксель уже существует, то "add_voxel" выдает ссылку на него и он рисуется.
  voxel_draw(add_voxel(P0));

  for (u_char side = 0; side < SIDES_COUNT; ++side) // Вернуть в рендер соседние воксели
    voxel_draw(get(i3d_shift(P0, side)));
}


///
/// \brief rdb::decrease
/// \param i - порядковый номер группы данных из буфера
///
/// \details Уменьшение объема с указанной стороны
///
void area::decrease(int i)
{
  if(i > (render_indices/indices_per_side) * bytes_per_side) return;

  // по номеру группы данных вычисляем ее адрес смещения в VBO
  GLsizeiptr offset = (i/vertices_per_side) * bytes_per_side;

  voxel* pVox = mVBO[offset];                 // По адресу смещения найдем бокс

  i3d P0 = pVox->Origin;                       // по ригу - опорную точку
  //u_char s0 = B->side_id_by_offset(offset); // Направление стороны бокса;

  for (u_char side = 0; side < SIDES_COUNT; ++side) // Убрать из рендера риги вокруг
    voxel_wipe(get(i3d_shift(P0, side)));

  voxel_wipe(pVox);                                 // убрать из рендера выбранный риг
  mArea.erase(pVox->Origin);                        // удалить риг из базы данных
  visibility_recalc(P0);                            // пересчитать видимость ригов вокруг
  for (u_char side = 0; side < SIDES_COUNT; ++side) // Вернуть в рендер соседние риги
    voxel_draw(get(i3d_shift(P0, side)));
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
void area::voxel_draw(voxel* pVox)
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
    mVBO[vbo_addr] = pVox;                          // добавить ссылку на воксель
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
void area::voxel_wipe(voxel* pVox)
{
  if(nullptr == pVox) return;
  if(!pVox->in_vbo) return;
  pVox->in_vbo = false;

  for(u_char side_id = 0; side_id < SIDES_COUNT; ++side_id)
  {
    GLsizeiptr dest = pVox->offset_read(side_id);  // адрес данных, которые будут перезаписаны
    if(dest < 0) continue;                     // -1 если сторона невидима - пропустить цикл
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
      voxel* pVoxMov = mVBO[free];        // Адрес вокселя, чьи данные переносятся.
      pVoxMov->offset_replace(free, dest);// изменить хранимый в боксе адрес размещения данных в VBO;
      mVBO[dest] = pVoxMov;               // заменить адрес блока в карте снипов.
    }
                                          // Если free == dest, то только удалить адрес с карты
    mVBO.erase(free);                     // удалить освободившийся элемент массива;
    render_indices -= indices_per_side;   // Уменьшить число точек рендера
  }
}


///
/// \brief Загрузка из базы данных в оперативную память блока пространства
///
/// \details  Формирование в оперативной памяти карты ригов (std::map) для
/// выбраной области пространства. Из этой карты берутся данные снипов,
/// размещаемых в VBO для рендера сцены.
///
void area::load_space(vbo_ext* v)
{
  pVBO = v;
  mArea.clear();
  mVBO.clear();
  render_indices = 0;

  int area_length = 20 * voxel_size;

  for (int x = -area_length; x < area_length + 1; x += voxel_size)
    for (int z = -area_length; z < area_length + 1; z += voxel_size)
  {
    add_voxel({x,-voxel_size * 2, z});
    add_voxel({x,-voxel_size, z});
    add_voxel({x, 0, z});
  }
}


///
/// \brief rdb::gen_rig
/// \param P
///
/// \details В указанной 3D точке генерирует воксель.
///
voxel* area::add_voxel(const i3d& P)
{
  voxel* pVox = get(P);                   // Если в этой точке уже есть воксель,
  if(nullptr != pVox) return pVox;        // то возвращается ссылка на него;
  mArea.emplace(P, std::pair<const i3d&, int>(P, voxel_size)); // иначе - создается новый.

  pVox = get(P);
  visibility_voxel_recalc(pVox);
  return pVox;
}


///
/// \brief rdb::visibility_recalc
/// \param R0
/// \details Пересчет видимости сторон вокса и его соседей вокруг него
///
void area::visibility_voxel_recalc(voxel* pVox0)
{
  if(nullptr == pVox0) return;

  voxel* pVox1 = nullptr;
  for (u_char side = 0; side < SIDES_COUNT; ++side)   // для их каждой стороны,
  {                                                   // получить
    pVox1 = get(i3d_shift(pVox0->Origin, side)); // ссылку на соседний риг.
    if(nullptr == pVox1) continue;                    // Если пусто - пропустить.
    pVox0->visible_check(side, pVox1);                // Проверить стыковку (видимость) стороны
  }                                                   // с каждым из боксов соседнего рига
}


///
/// \brief rdb::visibility_recalc
/// \param P0
/// \details Пересчет видимости ригов вокруг опорной точки
///
void area::visibility_recalc(i3d P0)
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
    pVox = get(i3d_shift(P0, side));
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
voxel* area::get(const i3d &P)
{
  try { return &mArea.at(P); }
  catch (...) { return nullptr; }
}

} //namespace
