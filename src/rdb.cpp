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
/// \brief rdb::caps_lock_toggle
/// \details переключить положение caps_lock
///
void rdb::caps_lock_toggle(void)
{
  caps_lock = !caps_lock;
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

  if(B->side_is_max(s0))         // Если бокс имеет максимальный размер с это стороны,
    P0 = i3d_shift(P0, s0, lod);  // получим точку для генерации нового рига рядом с ним.

  for (u_char side = 0; side < SIDES_COUNT; ++side) // Временно убрать из рендера риги
    rig_wipe(get(i3d_shift(P0, side, lod)));        // вокруг изменяемого (или нового) рига

  if(!B->side_is_max(s0))
  {                              // Если сторона не полная, то
    rig_wipe(Rig);               // стереть риг
    B->side_fill(s0);            // заполнить сторону
    visibility_recalc_rigs(Rig); // пересчитать видимость всех ригов вокруг
  }
  // Нарисовать в опорной точке риг. Если сторона была полная, то создается новый риг и
  // автоматически производится пересчет видимости сторон соседних ригов вокруг только
  // что созданного. Если риг уже существует, то "gen_rig" выдает ссылку на него и он рисуется.
  rig_draw(gen_rig(P0));

  for (u_char side = 0; side < SIDES_COUNT; ++side) // Вернуть в рендер соседние риги
    rig_draw(get(i3d_shift(P0, side, lod)));
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

  box* B = Visible[offset];                 // По адресу смещения найдем бокс
  rig* R = B->ParentRig;                    // по боксу - риг
  i3d P0 = R->Origin;                       // по ригу - опорную точку
  u_char s0 = B->side_id_by_offset(offset); // Направление стороны бокса;

  for (u_char side = 0; side < SIDES_COUNT; ++side) // Убрать из рендера риги вокруг
    rig_wipe(get(i3d_shift(P0, side, lod)));

  rig_wipe(R);                               // убрать из рендера выбранный риг

  if(!caps_lock)
  {
    MapRigs.erase(R->Origin); // удалить риг из базы данных
    visibility_recalc(P0);    // пересчитать видимость ригов вокруг
    for (u_char side = 0; side < SIDES_COUNT; ++side) // Вернуть в рендер соседние риги
      rig_draw(get(i3d_shift(P0, side, lod)));
    return;
  }

  u_char step = 50; // шаг уменьшения размера бокса
  if(B->reduce(s0, step))       // Попробовать уменьшить бокс.
  {                             // Если удачно, то
    visibility_recalc_rigs(R);  // пересчитать видимость
    rig_draw(R);                // и вернуть на экран.
  }
  else
  { // Если уменьшить не удалось (уже минимальный размер), то удалить его.
    for(size_t i = 0; i < R->Boxes.size(); ++i) if(B == &(R->Boxes[i]))
    {
      R->Boxes.erase(R->Boxes.begin() + i);
      i = R->Boxes.size();
    }

    if(R->Boxes.empty())        // В каждом риге может быть несколько боксов.
    {                           // Если их не осталость, то
      MapRigs.erase(R->Origin); // удалить риг из базы данных
      visibility_recalc(P0);    // пересчитать видимость ригов вокруг
    }
    else
    {                             // Если в риге еще остались боксы,
      visibility_recalc_rigs(R);  // то пересчитать видимость
      rig_draw(R);                // и вернуть риг на экран.
    }
  }

  for (u_char side = 0; side < SIDES_COUNT; ++side) // Вернуть в рендер соседние риги
    rig_draw(get(i3d_shift(P0, side, lod)));
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
void rdb::rig_draw(rig* R)
{
  if(nullptr == R) return;   // TODO: тут можно подгружать или дебажить
  if(R->in_vbo) return;      // Если данные уже в VBO - ничего не делаем

  GLsizeiptr vbo_addr = 0;
  GLfloat buffer[digits_per_snip];
  f3d P = R->vector();

  for(box& B0: R->Boxes) for(u_char side_id = 0; side_id < SIDES_COUNT; ++side_id)
  {
    if(!B0.side_fill_data(side_id, buffer, P)) continue;
    vbo_addr = VBO->append(buffer, bytes_per_snip);
    B0.offset_write(side_id, vbo_addr);                // записать в бокс адрес смещения VBO
    Visible[vbo_addr] = &B0;                           // добавить ссылку на бокс
    render_points += indices_per_snip;              // увеличить число точек рендера
  }
  R->in_vbo = true;
}


///
/// \brief rdb::remove_from_vbo
/// \param x
/// \param y
/// \param z
/// \details Стереть риг в рендере
///
void rdb::rig_wipe(rig* Rig)
{
  if(nullptr == Rig) return;
  if(!Rig->in_vbo) return;
  Rig->in_vbo = false;

  GLsizeiptr dest = 0;         // адрес смещения, где данные будут перезаписаны
  GLsizeiptr free = 0;         // адрес блока в "хвоста" VBO, который будет освобожден

  for(box& B: Rig->Boxes) for(u_char side_id = 0; side_id < SIDES_COUNT; ++side_id)
  {
    dest = B.offset_read(side_id);             // адрес данных, которые будут перезаписаны
    if(dest < 0) continue;                     // -1 если сторона невидима - пропустить цикл
    B.offset_write(side_id, -1);
    free = VBO->remove(dest, bytes_per_snip);  // убрать снип из VBO и получить адрес смещения
                                               // в VBO с которого данные были перенесены на dest

    if(free == 0)                              // Если VBO пустой
    {
      Visible.clear();
      render_points = 0;
      return;
    }

    if (free != dest)                     // Если удаляемый блок данных не в конце, то на его
    {                                     // место переписываются данные с адреса free.
      box* BMoved = Visible[free];        // Адрес бокса, чьи данные переносятся.
      BMoved->offset_replace(free, dest); // изменить хранимый в боксе адрес размещения данных в VBO;
      Visible[dest] = BMoved;             // заменить адрес блока в карте снипов.
    }
                                          // Если free == dest, то только удалить адрес с карты
    Visible.erase(free);                  // удалить освободившийся элемент массива;
    render_points -= indices_per_snip;    // Уменьшить число точек рендера
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

  //Загрузка из базы данных
  //cfg::DataBase.rigs_loader(MapRigs, From, To);
  std::srand(std::time(nullptr));

  int side_length = 20;
  for (int x = -side_length; x < side_length+1; ++x)
    for (int z = -side_length; z < side_length+1; ++z)
  {
    gen_rig({x,-1, z});
    gen_rig({x, 0, z});
    gen_rig({x, 1, z}, 255, 255 - static_cast<u_char>(std::rand() % 30), 255);
  }
}


///
/// \brief rdb::gen_rig
/// \param P
///
/// \details В указанной точке генерирует однобоксовый риг. Можно указать
/// размеры сторон бокса. По-умолчанию они устанавливаются = 255
///
rig* rdb::gen_rig(const i3d& P, u_char lx, u_char ly, u_char lz)
{
  rig* R = get(P);            // Если в этой точке уже есть риг,
  if(nullptr != R) return R;  // то возвращается ссылка на него

  MapRigs[P] = rig{P};
  R = get(P);

  R->Boxes.push_back(box{ {0, 0, 0}, {lx, ly, lz}, R});
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
