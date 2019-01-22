/*
 *
 * file: rdb.cpp
 *
 * Управление элементами 3D пространства
 *
 */

#include "rdb.hpp"
#include "config.hpp"

#define DBG std::cout

namespace tr
{

#define R0y0 R0->SideYp.front().data[Y + ROW_SIZE * 0]
#define R0y1 R0->SideYp.front().data[Y + ROW_SIZE * 1]
#define R0y2 R0->SideYp.front().data[Y + ROW_SIZE * 2]
#define R0y3 R0->SideYp.front().data[Y + ROW_SIZE * 3]

#define R1y0 R1->SideYp.front().data[Y + ROW_SIZE * 0]
#define R1y1 R1->SideYp.front().data[Y + ROW_SIZE * 1]
#define R1y2 R1->SideYp.front().data[Y + ROW_SIZE * 2]
#define R1y3 R1->SideYp.front().data[Y + ROW_SIZE * 3]


///
/// \brief rdb::lay_direction
/// \param N
/// \return направление вектора
///
LAY_NAME rdb::lay_direction(const glm::vec4& N)
{
  if (( abs(N.x) > abs(N.y) ) && ( abs(N.x) > abs(N.z) ) &&
         ( (N.x) >= 0.0f )) return LAY_XP;
  else if (( abs(N.x) > abs(N.y) ) && ( abs(N.x) > abs(N.z) ) &&
         ( (N.x) < 0.0f )) return LAY_XN;
  else if (( abs(N.y) > abs(N.x) ) && ( abs(N.y) > abs(N.z) ) &&
         ( (N.y) >= 0.0f )) return LAY_YP;
  else if (( abs(N.y) > abs(N.x) ) && ( abs(N.y) > abs(N.z) ) &&
         ( (N.y) < 0.0f )) return LAY_YN;
  else if (( abs(N.z) > abs(N.x) ) && ( abs(N.z) > abs(N.y) ) &&
         ( (N.z) >= 0.0f )) return LAY_ZP;
  else if (( abs(N.z) > abs(N.x) ) && ( abs(N.z) > abs(N.y) ) &&
         ( (N.z) < 0.0f )) return LAY_ZN;
  else return LAYS_COUNT;
}


///
/// \brief rdb::is_top
/// \param V
/// \param n
/// \return У всех 4-х векторов нет дробной части в указанной по индексу (n) координате
///
bool rdb::is_top(const std::array<glm::vec4, 4>& V, size_t n)
{
  return ((nearbyint(V[0][n]) == V[0][n]) &&
          (nearbyint(V[1][n]) == V[1][n]) &&
          (nearbyint(V[2][n]) == V[2][n]) &&
          (nearbyint(V[3][n]) == V[3][n]) );
}


///
/// \brief rdb::snip_analise
/// \param S
///
/// \details Установка значений ".lay" и "Origin"
/// на основе анализ положения снипа в пространстве
///
void rdb::snip_analyze(snip_ext& S)
{
  std::array<glm::vec4, 4> V {};
  glm::vec4 NS { 0.0f, 0.0f, 0.0f, 0.0f };

  for(size_t i = 0; i <4; i++)
  {
    V[i] = S.vertex_coord(i);
    NS += S.vertex_normal(i);
  }

  S.lay = lay_direction(NS);
  switch (S.lay)
  {
    case LAY_XP:
      S.Origin = V[3];
      if(is_top(V, 0)) S.Origin.y -= lod;
      break;
    case LAY_XN:
      S.Origin = V[2];
      break;
    case LAY_YP:
      S.Origin = V[3];
      if(is_top(V, 1)) S.Origin.y -= lod;
      break;
    case LAY_YN:
      S.Origin = V[0];
      break;
    case LAY_ZP:
      S.Origin = V[2];
      if(is_top(V, 2)) S.Origin.y -= lod;
      break;
    case LAY_ZN:
      S.Origin = V[3];
      break;
    case LAYS_COUNT:
#ifndef NDEBUG
      info("Undefined LAY direction.");
#endif
      break;
  }

  S.Origin.x = floor(S.Origin.x);
  S.Origin.y = floor(S.Origin.y);
  S.Origin.z = floor(S.Origin.z);
}


///
/// \brief rdb::increase
/// \param i
///
/// \details Добавление объема к указанной стороне
///
void rdb::increase(unsigned int i)
{
  //debug
  //info("increase call");

  if(i > (render_points/indices_per_snip) * bytes_per_snip) return;

  GLsizeiptr offset = (i/vertices_per_snip) * bytes_per_snip; // + bytes_per_vertex;
  snip_ext S{};
  VBO->data_get(offset, bytes_per_snip, S.data); // считать из VBO данные снипа
  snip_analyze(S);
  switch (S.lay)
  {
    case LAY_XP:
      add_xp(S.Origin);
      break;
    case LAY_XN:
      add_xn(S.Origin);
      break;
    case LAY_YP:
      add_yp(S.Origin);
      break;
    case LAY_YN:
      add_yn(S.Origin);
      break;
    case LAY_ZP:
      add_zp(S.Origin);
      break;
    case LAY_ZN:
      add_zn(S.Origin);
      break;
    case LAYS_COUNT:
      break;
  }
}


///
/// \brief rdb::decrease
/// \param i
///
/// \details Уменьшение объема с указанной стороны
///
void rdb::decrease(unsigned int i)
{
  if(i > (render_points/indices_per_snip) * bytes_per_snip) return;

  GLsizeiptr offset = (i/vertices_per_snip) * bytes_per_snip; // + bytes_per_vertex;
  snip_ext S{};
  VBO->data_get(offset, bytes_per_snip, S.data); // считать из VBO данные снипа
  snip_analyze(S);                               //
  switch (S.lay)
  {
    case LAY_XP:
      sub_xp(S.Origin);
      break;
    case LAY_XN:
      sub_xn(S.Origin);
      break;
    case LAY_YP:
      sub_yp(S.Origin);
      break;
    case LAY_YN:
      sub_yn(S.Origin);
      break;
    case LAY_ZP:
      sub_zp(S.Origin);
      break;
    case LAY_ZN:
      sub_zn(S.Origin);
      break;
    case LAYS_COUNT:
      break;
  }
}


///
/// Добавление в графический буфер элементов рига
///
void rdb::rig_display(rig* R)
{
  if(nullptr == R) return;   // TODO: тут можно подгружать или дебажить
  if(R->in_vbo) return;      // Если данные уже в VBO - ничего не делаем

  f3d Point = {
    static_cast<float>(R->Origin.x) + R->shift[SHIFT_X],
    static_cast<float>(R->Origin.y) + R->shift[SHIFT_Y],
    static_cast<float>(R->Origin.z) + R->shift[SHIFT_Z]  // TODO: еще есть поворот и zoom
  };

  side_display(R->SideXp, Point);
  side_display(R->SideXn, Point);
  side_display(R->SideYp, Point);
  side_display(R->SideYn, Point);
  side_display(R->SideZp, Point);
  side_display(R->SideZn, Point);

  R->in_vbo = true;
}


///
/// \brief rdb::place_snip
/// \param Side
/// \param Point
///
/// \brief
/// Добавляет данные снипов указанной стороны в конец VBO
///
/// \details
/// Координаты вершин снипов хранятся в нормализованом виде, поэтому перед
/// отправкой в VBO все данные снипа копируются во временный кэш, где
/// координаты вершин пересчитываются с учетом координат (TODO: сдвига и
/// поворота рига-контейнера), после чего данные записываются в VBO.
/// Адрес смещения блока данных в VBO запоминается в переменной снипа.
///
void rdb::side_display(std::vector<snip>& Side, const f3d& Point)
{
  for(snip& Snip: Side)
  {
    GLfloat cache[digits_per_snip] = {0.0f};
    memcpy(cache, Snip.data, bytes_per_snip);

    // Расчитать абсолютные координаты всех вершин снипа
    for(size_t n = 0; n < vertices_per_snip; n++)
    {
      cache[ROW_SIZE * n + X] += Point.x;
      cache[ROW_SIZE * n + Y] += Point.y;
      cache[ROW_SIZE * n + Z] += Point.z;
    }

    Snip.data_offset = VBO->data_append(cache,  bytes_per_snip); // записать расположение в VBO
    render_points += indices_per_snip;                           // увеличить число точек рендера
    VisibleSnips[Snip.data_offset] = &Snip;                      // добавить ссылку
  }
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

  side_wipeoff(Rig->SideYp);
  side_wipeoff(Rig->SideYn);
  side_wipeoff(Rig->SideZp);
  side_wipeoff(Rig->SideZn);
  side_wipeoff(Rig->SideXp);
  side_wipeoff(Rig->SideXn);

  Rig->in_vbo = false;
}


///
/// \brief side_wipeoff
/// \param Side
///
/// \details Удаление из VBO данных указанной стороны
///
void rdb::side_wipeoff(std::vector<snip>& Side)
{
  GLsizeiptr target = 0;         // адрес смещения, где данные будут перезаписаны
  GLsizeiptr moving = 0;         // адрес снипа с "хвоста" VBO, который будет перемещен на target

  if(Side.empty()) return;
  for(auto& Snip: Side)
  {
    target = Snip.data_offset;                    // адрес снипа, подлежащий удалению
    moving = VBO->remove(target, bytes_per_snip); // убрать снип из VBO и получить адрес крайнего

    if(moving == 0)                               // Если VBO пустой
    {
      VisibleSnips.clear();
      render_points = 0;
      return;
    }
    else if (moving == target)                     // Если удаляемый снип оказался в конце активной
    {                                              // части VBO, то только удалить его с карты
      VisibleSnips.erase(target);
    }
    else                                           // Если удаляемый снип не в конце, то на его место
    {                                              // ставится крайний снип в конце VBO:
      VisibleSnips[target] = VisibleSnips[moving]; // - заменить блок в карте снипов,
#ifndef NDEBUG
      if(VisibleSnips[target]->data_offset != moving) info("Err data_offset on rdb::side_vbo_remove");
#endif
      VisibleSnips[target]->data_offset = target;  // - изменить адрес размещения в VBO у перемещенного снипа,
      VisibleSnips.erase(moving);                  // - удалить освободившийся элемент
    }
    render_points -= indices_per_snip;             // Уменьшить число точек рендера
  }
}


///
/// \brief rdb::side_make_snip
/// \param v координаты 4-х опорных точек
/// \param S формируемый снип
/// \param n направление нормали стороны
/// \details Формирование снипа для боковой стороны рига
///
void rdb::side_make_snip(const std::array<glm::vec4, 4>& v, snip& S, const glm::vec3& n)
{
  for(size_t i = 0; i < v.size(); ++i)
  {
    S.data[ROW_SIZE * i + X] = v[i].x;
    S.data[ROW_SIZE * i + Y] = v[i].y;
    S.data[ROW_SIZE * i + Z] = v[i].z;
    S.data[ROW_SIZE * i + W] = 1.0f;

    S.data[ROW_SIZE * i + NX] = n.x;
    S.data[ROW_SIZE * i + NY] = n.y;
    S.data[ROW_SIZE * i + NZ] = n.z;
    S.data[ROW_SIZE * i + NW] = 0.0f;
  }
}


///
/// \brief rdb::make_Yn
/// \param SideYn
///
void rdb::make_Yn(std::vector<snip>& SideYn)
{
  snip S{};                                       // Стандартный снип для стенки
  for(size_t i = 0; i < 4; ++i)                   // настроить нормали вниз
  {
    S.data[ROW_SIZE * i + NX] =  0.f;
    S.data[ROW_SIZE * i + NY] = -1.f;
    S.data[ROW_SIZE * i + NZ] =  0.f;
  }
  S.texture_set( AppWin.texYn.u, AppWin.texYn.v); // настроить текстуру
  SideYn.clear();
  SideYn.push_back(S);                            // заполнить сторону
}


///
/// \brief rdb::make_Xp
/// \param Top - верхний снип
/// \param Side - боковой снип, который перестраивается
/// \param y2 - высота вершины v2
/// \param y3 - высота вершины v3
///
/// \details Построение стороны +X одного рига. Вызов производится после проверки того, что
/// данную сторону видно - нет соседнего блока, либо он ниже.
///
void rdb::make_Xp(std::vector<snip>& VTop, std::vector<snip>& VSide, float y2, float y3)
{
  std::array<glm::vec4, 4> V {};

  if(VTop.empty())
  {
    V[0] = {lod, lod, 0, 1};
    V[1] = {lod, lod, lod, 1};
  } else
  {
    V[0] = VTop.front().vertex_coord(2);
    V[1] = VTop.front().vertex_coord(1);
  }

  V[2] = V[1]; V[2].y = y2;
  V[3] = V[0]; V[3].y = y3;

  snip S{};
  side_make_snip(V, S, {1.f, 0.f, 0.f});
  S.texture_fragment( AppWin.texXp.u, AppWin.texXp.v,
                     {V[0].z, V[0].y, V[1].z, V[1].y, V[2].z, V[2].y, V[3].z, V[3].y} );
  VSide.clear();
  VSide.push_back(S);
}


///
/// \brief rdb::make_Xn
/// \param Top - верхний снип
/// \param Side - боковой снип, который перестраивается
/// \param y2 - высота вершины v2
/// \param y3 - высота вершины v3
/// \details Построение стороны -X. Вызов производится после проверки того, что
/// данную сторону видно - нет соседнего блока, либо он ниже.
///
void rdb::make_Xn(std::vector<snip>& VTop, std::vector<snip>& VSide, float y2, float y3)
{
  std::array<glm::vec4, 4> V {};

  if(VTop.empty())
  {
    V[0] = {0, lod, lod, 1};
    V[1] = {0, lod, 0, 1};
  } else
  {
    V[0] = VTop.front().vertex_coord(0);
    V[1] = VTop.front().vertex_coord(3);
  }

  V[2] = V[1]; V[2].y = y2;
  V[3] = V[0]; V[3].y = y3;

  snip S{};
  side_make_snip(V, S, {-1.f, 0.f, 0.f});
  S.texture_fragment( AppWin.texXn.u, AppWin.texXn.v,
                     {lod-V[0].z, V[0].y, lod-V[1].z, V[1].y, lod-V[2].z, V[2].y, lod-V[3].z, V[3].y} );
  VSide.clear();
  VSide.push_back(S);
}


///
/// \brief rdb::make_Zn
/// \param Top - верхний снип
/// \param Side - боковой снип, который перестраивается
/// \param y2 - высота вершины v2
/// \param y3 - высота вершины v3
/// \details Построение стороны -Z. Вызов производится после проверки того, что
/// данную сторону видно (нет соседнего блока, либо он ниже).
///
void rdb::make_Zn(std::vector<snip>& VTop, std::vector<snip>& VSide, float y2, float y3)
{
  std::array<glm::vec4, 4> V {};

  if(VTop.empty())
  {
    V[0] = {0, lod, 0, 1};
    V[1] = {lod, lod, 0, 1};
  } else
  {
    V[0] = VTop.front().vertex_coord(3);
    V[1] = VTop.front().vertex_coord(2);
  }

  V[2] = V[1]; V[2].y = y2;
  V[3] = V[0]; V[3].y = y3;

  snip S{};
  side_make_snip(V, S, {0.f, 0.f, -1.f});
  S.texture_fragment( AppWin.texZn.u, AppWin.texZn.v,
                     {V[0].x, V[0].y, V[1].x, V[1].y, V[2].x, V[2].y, V[3].x, V[3].y} );
  VSide.clear();
  VSide.push_back(S);
}


///
/// \brief rdb::make_Zp
/// \param Top - верхний снип
/// \param Side - боковой снип, который перестраивается
/// \param y2 - высота вершины v2
/// \param y3 - высота вершины v3
/// \details Построение стороны +Z. Вызов производится после проверки того, что
/// данную сторону видно (нет соседнего блока, либо он ниже).
///
void rdb::make_Zp(std::vector<snip>& VTop, std::vector<snip>& VSide, float y2, float y3)
{
  std::array<glm::vec4, 4> V {};

  if(VTop.empty())
  {
    V[0] = {lod, lod, lod, 1};
    V[1] = {0, lod, lod, 1};
  } else
  {
    V[0] = VTop.front().vertex_coord(1);
    V[1] = VTop.front().vertex_coord(0);
  }

  V[2] = V[1]; V[2].y = y2;
  V[3] = V[0]; V[3].y = y3;

  snip S{};
  side_make_snip(V, S, {0.f, 0.f, 1.f});
  S.texture_fragment( AppWin.texZp.u, AppWin.texZp.v,
                     {lod-V[0].x, V[0].y, lod-V[1].x, V[1].y, lod-V[2].x, V[2].y, lod-V[3].x, V[3].y} );
  VSide.clear();
  VSide.push_back(S);
}


///
/// \brief rdb::set_Zp
/// \param R0, R1
/// \details Выбор параметров для построения стенки +Z
///
void rdb::set_Zp(rig* R0, rig* R1)
{
#ifndef NDEBUG
  if(nullptr == R0) ERR("Call rdb::set_Zp with nullptr");
  if(R0->SideYp.empty()) ERR("Call rdb::set_Zp with R0->SideYp.empty()");
#endif

  R0->SideZp.clear();

  if(nullptr == R1) // Если рядом нет блока, то +Z стенка строится до низа рига
  {
    make_Zp( R0->SideYp, R0->SideZp, 0.f, 0.f );
    return;
  }

  rig_wipeoff(R1);
  R1->SideZn.clear();    // убрать стенку -Z соседнего блока (если она есть)

  // Если соседний блок без верха или выше, то построить -Z стенку соседнего блока
  if( R1->SideYp.empty() || ((R0y1 < R1y2) && (R0y0 < R1y3)) )
  {  make_Zn(R1->SideYp, R1->SideZn, R0y1, R0y0); }
  else // иначе - построить свою +Z
  {  make_Zp( R0->SideYp, R0->SideZp, R1y3, R1y2 ); }

  rig_display(R1);
}


///
/// \brief rdb::set_Zn
/// \param R0, R1
/// \details Выбор параметров для построения стенки -Z
///
void rdb::set_Zn(rig* R0, rig* R1)
{
#ifndef NDEBUG
  if(nullptr == R0) ERR("Call rdb::set_Zn with nullptr");
  if(R0->SideYp.empty()) ERR("Call rdb::set_Zn with R0->SideYp.empty()");
#endif

  R0->SideZn.clear();

  if(nullptr == R1) // Если рядом нет блока, то -Z строится до низа рига
  {
    make_Zn( R0->SideYp, R0->SideZn, 0.f, 0.f );
    return;
  }

  rig_wipeoff(R1);
  R1->SideZp.clear();    // убрать +Z соседнего блока (если есть)

  // Если соседний блок без верха или выше, то построить +Z соседнего блока
  if( R1->SideYp.empty() || ((R0y3 < R1y0) && (R0y2 < R1y1)) )
  {  make_Zp( R1->SideYp, R1->SideZp, R0y3, R0y2 ); }
  else // иначе - обновить -Z
  {  make_Zn( R0->SideYp, R0->SideZn, R1y1, R1y0 ); }

  rig_display(R1);
}


///
/// \brief rdb::set_Xp
/// \param R0, R1
/// \details Выбор параметров для построения стенки +X
///
void rdb::set_Xp(rig* R0, rig* R1)
{
#ifndef NDEBUG
  if(nullptr == R0) ERR("Call rdb::set_Xp with nullptr");
  if(R0->SideYp.empty()) ERR("Call rdb::set_Xp with R0->SideYp.empty()");
#endif

  R0->SideXp.clear();

  if(nullptr == R1) // Если рядом нет блока, то +X строится до низа рига
  {
    make_Xp( R0->SideYp, R0->SideXp, 0.f, 0.f );
    return;
  }

  rig_wipeoff(R1);
  R1->SideXn.clear();  // стенку -X соседнего блока убираем

  // Если соседний блок без верха или выше, то строим -X соседнего блока
  if( R1->SideYp.empty() || ((R0y2 < R1y3) && (R0y1 < R1y0)) )
  {  make_Xn( R1->SideYp, R1->SideXn, R0y2, R0y1 ); }
  else // иначе - строим свою +X
  {  make_Xp( R0->SideYp, R0->SideXp, R1y0, R1y3 ); }

  rig_display(R1);
}


///
/// \brief rdb::set_Xn
/// \param R0, R1
/// \details Выбор параметров для построения стенки -X
///
void rdb::set_Xn(rig* R0, rig* R1)
{
#ifndef NDEBUG
  if(nullptr == R0) ERR("Call rdb::set_Xn with nullptr");
  if(R0->SideYp.empty()) ERR("Call rdb::set_Xn with R0->SideYp.empty()");
#endif

  R0->SideXn.clear();

  if(nullptr == R1) // Если рядом нет блока, то -X строится до низа рига
  {
    make_Xn( R0->SideYp, R0->SideXn, 0.f, 0.f );
    return;
  }

  rig_wipeoff(R1);
  R1->SideXp.clear(); // +X соседнего блока убрать

  // Если соседний блок без верха или выше, то построить +X соседнего блока
  if( R1->SideYp.empty() || ((R0y0 < R1y1) && (R0y3 < R1y2)) )
  {  make_Xp( R1->SideYp, R1->SideXp, R0y0, R0y3 ); }
  else // иначе - построить свою -X
  {  make_Xn( R0->SideYp, R0->SideXn, R1y2, R1y1 ); }

  rig_display(R1);
}


///
/// \brief rdb::sides_set
/// \param R
/// \details Построение боковых сторон. Перед вызовом методов для
/// боковых сторон проиводится проверка 4-х верхних ребер рига чтобы они не
/// пересекались с соответствующими ребрами соседних ригов. В случае, если
/// из пары вершин любого ребра одна расположена выше соответствующей
/// соседней вершины, а вторая ниже (возникло пересечение), то производим
/// коррекцию высоты нижней вершины ребра основного рига - поднимаем ее до
/// высоты соответствующей вершины соседнего рига. При этом пересекающиеся
/// ранее ребра выстраиваются в форме треугольника.
///
/// Так как при этом изменяется положение верхней стороны, то это необходимо
/// проделать до вызова методов построения боковых сторон. Иначе может возникнуть
/// ситуация когда одна из вершин будет приподнята после того, как сопряженная
/// с ней сторона уже построена. Это приведет к образованию в данной стороне
/// треугольного отверстия.
///
void rdb::sides_set(rig* R0)
{
  rig* R1 = nullptr;
  i3d pOr {0,0,0};

  pOr = {R0->Origin.x, R0->Origin.y, R0->Origin.z + lod};   // +Z
  R1 = get(pOr);
  if(nullptr != R1)
  {
    if(!R1->SideYp.empty())
    {
      if((R0y1 < R1y2) && (R0y0 > R1y3)) R0y1 = R1y2;
      if((R0y0 < R1y3) && (R0y1 > R1y2)) R0y0 = R1y3;
    }
  } else { if(R0->SideZp.empty()) MapRigs[pOr] = rig{pOr}; }

  pOr = {R0->Origin.x, R0->Origin.y, R0->Origin.z - lod};   // -Z
  R1 = get({R0->Origin.x, R0->Origin.y, R0->Origin.z - lod});
  if(nullptr != R1)
  {
    if(!R1->SideYp.empty())
    {
      if((R0y3 < R1y0) && (R0y2 > R1y1)) R0y3 = R1y0;
      if((R0y2 < R1y1) && (R0y3 > R1y0)) R0y2 = R1y1;
    }
  } else { if(R0->SideZn.empty()) MapRigs[pOr] = rig{pOr}; }

  pOr = {R0->Origin.x + lod, R0->Origin.y, R0->Origin.z};   // +X
  R1 = get({R0->Origin.x + lod, R0->Origin.y, R0->Origin.z});
  if(nullptr != R1)
  {
    if(!R1->SideYp.empty())
    {
      if((R0y2 < R1y3) && (R0y1 > R1y0)) R0y2 = R1y3;
      if((R0y1 < R1y0) && (R0y2 > R1y3)) R0y1 = R1y0;
    }
  } else { if(R0->SideXp.empty()) MapRigs[pOr] = rig{pOr}; }

  pOr = {R0->Origin.x - lod, R0->Origin.y, R0->Origin.z};   // -X
  R1 = get({R0->Origin.x - lod, R0->Origin.y, R0->Origin.z});
  if(nullptr != R1)
  {
    if(!R1->SideYp.empty())
    {
      if((R0y0 < R1y1) && (R0y3 > R1y2)) R0y0 = R1y1;
      if((R0y3 < R1y2) && (R0y0 > R1y1)) R0y3 = R1y2;
    }
  } else { if(R0->SideXn.empty()) MapRigs[pOr] = rig{pOr}; }

  set_Zp( R0, get({R0->Origin.x, R0->Origin.y, R0->Origin.z + lod}) );
  set_Zn( R0, get({R0->Origin.x, R0->Origin.y, R0->Origin.z - lod}) );
  set_Xp( R0, get({R0->Origin.x + lod, R0->Origin.y, R0->Origin.z}) );
  set_Xn( R0, get({R0->Origin.x - lod, R0->Origin.y, R0->Origin.z}) );
}


///
/// \brief rdb::append_rig_Yp
/// \param Pt
/// \brief Добавить сверху новый риг
///
void rdb::append_rig_Yp(const i3d& Pt)
{
  MapRigs.emplace(std::pair(Pt, rig{}));
  MapRigs[Pt].Origin = Pt;
  snip S = {};
  for (size_t i = Y; i < digits_per_snip; i += ROW_SIZE) S.data[i] = 0.25f;
  S.texture_set(AppWin.texYp.u, AppWin.texYp.v);

  MapRigs[Pt].SideYp.push_back(S);
  sides_set(&MapRigs[Pt]);
  rig_display(&MapRigs[Pt]);
}


///
/// \brief rdb::add_y
/// \details Увеличение размера по координате Y
///
void rdb::add_yp(const i3d& Pt)
{
  rig *R = get(Pt);         //1. Выбрать целевой риг
  if(nullptr == R)
  {
    info ("Error: call rdb::add_y for nullptr point: "
          + std::to_string(Pt.x) + ", " + std::to_string(Pt.y) + ", " + std::to_string(Pt.y) );
    return;
  }

  if(R->SideYp.empty()) return;
  rig_wipeoff(R); // убрать риг из графического буфера

  snip &S = R->SideYp.front();
  if((S.data[Y + ROW_SIZE * 0] == 1.00f) &&
     (S.data[Y + ROW_SIZE * 1] == 1.00f) &&
     (S.data[Y + ROW_SIZE * 2] == 1.00f) &&
     (S.data[Y + ROW_SIZE * 3] == 1.00f))
  {
    R->SideYp.clear();
    append_rig_Yp({Pt.x, Pt.y + lod, Pt.z});
    rig_display(R);
    return;
  }

  float y = 0.f;
  // найти вершину с максимальным значением Y
  for (size_t i = Y; i < digits_per_snip; i += ROW_SIZE) y = std::max(y, S.data[i]);

  // округлить до ближайшей сверху четверти
  if(y >= 0.75f) y = 1.00f;
  else if (y >= 0.50f) y = 0.75f;
  else if (y >= 0.25f) y = 0.50f;
  else y = 0.25f;

  // выровнять все вершины по выбранной высоте
  for (size_t i = Y; i < digits_per_snip; i += ROW_SIZE) S.data[i] = y;

  // настроить нормали
  for(size_t i = 0; i < 4; ++i)
  {
    S.data[ROW_SIZE * i + NX] = 0.f;
    S.data[ROW_SIZE * i + NY] = 1.f;
    S.data[ROW_SIZE * i + NZ] = 0.f;
    S.data[ROW_SIZE * i + NW] = 0.f;
  }

  sides_set(R);   // настроить боковые стороны
  rig_display(R);   // записать модифицированый риг в графический буфер
}


void rdb::add_yn(const i3d&) { info("add YP"); }
void rdb::add_xn(const i3d&) { info("add XN"); }
void rdb::add_xp(const i3d&) { info("add XP"); }
void rdb::add_zn(const i3d&) { info("add ZN"); }
void rdb::add_zp(const i3d&) { info("add ZP"); }

void rdb::sub_yn(const i3d&) { info("sub YP"); }
void rdb::sub_xn(const i3d&) { info("sub XN"); }
void rdb::sub_xp(const i3d&) { info("sub XP"); }
void rdb::sub_zn(const i3d&) { info("sub ZN"); }
void rdb::sub_zp(const i3d&) { info("sub ZP"); }


///
/// \brief rdb::make_Yp
/// \param SideYp
///
void rdb::make_Yp(std::vector<snip>& SideYp)
{
  SideYp.clear();
  snip S {};
  for (size_t i = Y; i < digits_per_snip; i += ROW_SIZE)  S.data[i] = 1.f;
  for (size_t i = NY; i < digits_per_snip; i += ROW_SIZE) S.data[i] = 1.f;
  S.texture_set(AppWin.texYp.u, AppWin.texYp.v);
  SideYp.push_back(S);
}


///
/// \brief rdb::remove_rig_Yp
/// \param Pt
/// \details Удаляет риг в указанной точке пространства
/// и обновляет стороны ригов, примыкающих к данной точке.
///
/// Перед удалением производится проверка дла каждой стороны условий:
/// 1. Если этой стороны нет в буфере, то всегда создается ответная сторона
/// примыкающего рига. Если такого рига в базе данных нет - его следует создать.
/// 2. Если проверяемая сторона имеется, то при наличии примыкающего
/// рига его ответная сторона обновляется; если примыкающего рига в БД нет, то
/// новый не создается.
///
void rdb::remove_rig(const i3d& pRm)
{
  rig* RigRm = get(pRm);                     // Указатель на удаляемый риг

  rig* RigTst = nullptr;                     // Указатель на соседний риг
  i3d  pTst {0,0,0};                         // Адрес соседнего рига

  // Y + lod
  pTst = {pRm.x, pRm.y + lod, pRm.z};
  RigTst = get(pTst);                        // Указатель на соседний риг
  if(RigRm->SideYp.empty())                  // Если удаляемой стенки нет,
  {                                          // то рядом есть риг и надо нарисовать его стенку
    if(nullptr == RigTst)
    {                                        // Если соседа в БД нет,
      MapRigs[pTst] = rig{pTst};             //  то его следует создать
      RigTst = get(pTst);                    // Получить указатель на созданного соседа
    }
    side_wipeoff(RigTst->SideYn);            // Очистить изображение стороны(если есть)
    make_Yn(RigTst->SideYn);                 // Построить нижнюю стенку
    side_display(RigTst->SideYp, pTst);      // Записать в графический буфер
    RigTst->in_vbo = true;
  }
  else // Если у удаляемого рига есть вверху стенка, то проверить наличие соседа,
  {    // если сосед присутствует, то обновить его нижнюю стенку
    if(nullptr != RigTst)
    {
      side_wipeoff(RigTst->SideYn);          // Очистить изображение стороны(если есть)
      make_Yn(RigTst->SideYn);               // Построить нижнюю стенку
      side_display(RigTst->SideYp, pTst);    // Записать в графический буфер
      RigTst->in_vbo = true;
    }
  }

  // Y - lod
  pTst = {pRm.x, pRm.y - lod, pRm.z};
  RigTst = get(pTst);                              // Указатель на риг снизу
  if(RigRm->SideYn.empty())                        // Если удаляемой стенки нет, то надо
  {                                                // нарисовать верхнюю стенку нижнего рига
    if(nullptr == RigTst)
    {                                              // Если соседа в БД нет,
      MapRigs[pTst] = rig{pTst};                   //  то его следует создать
      RigTst = get(pTst);                          // Получить указатель на созданного соседа
    }
    side_wipeoff(RigTst->SideYp);                  // Очистить изображение стороны(если есть)
    make_Yp(RigTst->SideYp);                       // Построить нижнюю стенку
    side_display(RigTst->SideYp, pTst);            // Записать в графический буфер
    RigTst->in_vbo = true;
  }
  else // Если у удаляемого рига есть вверху стенка, то проверить наличие соседа,
  {    // если сосед присутствует, то обновить его нижнюю стенку
    if(nullptr != RigTst)
    {
      side_wipeoff(RigTst->SideYp);                 // Очистить изображение стороны(если есть)
      make_Yp(RigTst->SideYp);                      // Построить нижнюю стенку
      side_display(RigTst->SideYp, pTst);           // Записать в графический буфер
      RigTst->in_vbo = true;
    }
  }


  // Х + lod
  RigTst = get({pRm.x + lod, pRm.y, pRm.z});
  if(nullptr != RigTst)
  {
    rig_wipeoff(RigTst);  // убрать риг из графического буфера
    make_Xn( RigTst->SideYp, RigTst->SideXn, 0.f, 0.f );
    rig_display(RigTst);     // записать модифицированый риг в графический буфер
  }

  // X - lod
  RigTst = get({pRm.x - lod, pRm.y, pRm.z});
  if(nullptr != RigTst)
  {
    rig_wipeoff(RigTst);  // убрать риг из графического буфера
    make_Xp( RigTst->SideYp, RigTst->SideXp, 0.f, 0.f );
    rig_display(RigTst);     // записать модифицированый риг в графический буфер
  }

  // Z + lod
  RigTst = get({pRm.x, pRm.y, pRm.z + lod});
  if(nullptr != RigTst)
  {
    rig_wipeoff(RigTst);  // убрать риг из графического буфера
    make_Zn( RigTst->SideYp, RigTst->SideZn, 0.f, 0.f );
    rig_display(RigTst);     // записать модифицированый риг в графический буфер
  }

  // Z - lod
  RigTst = get({pRm.x, pRm.y, pRm.z - lod});
  if(nullptr != RigTst)
  {
    rig_wipeoff(RigTst);  // убрать риг из графического буфера
    make_Zp( RigTst->SideYp, RigTst->SideZp, 0.f, 0.f );
    rig_display(RigTst);     // записать модифицированый риг в графический буфер
  }

  rig_wipeoff(RigRm);
  MapRigs.erase(pRm);             // Удалить риг из БД
}


///
/// \brief rdb::sub_yp
///
/// \details Уменьшение высоты рига на четверть. Если высота
/// меньше четверти (0,25*lod), то этот риг удаляется.
///
void rdb::sub_yp(const i3d& Pt)
{
  rig *R = get(Pt);         //1. Выбрать целевой риг

  if(nullptr == R)
  {
    info("Error: call rdb::sub_yp for nullptr point");
    return;
  }

  if(R->SideYp.empty()) return;

  rig_wipeoff(R); // убрать риг из графического буфера

  snip& S = R->SideYp.front();

  if((S.data[Y + ROW_SIZE * 0] <= 0.25f) ||
     (S.data[Y + ROW_SIZE * 1] <= 0.25f) ||
     (S.data[Y + ROW_SIZE * 2] <= 0.25f) ||
     (S.data[Y + ROW_SIZE * 3] <= 0.25f))
  {                                         // Если одна из вершин данного рига
    remove_rig(Pt);                         // расположена ниже у=0.25, то этот
    return;                                 // риг просто удаляется из БД
  }

  float y = 1.f; // найти вершину с минимальным значением Y
  for (size_t i = Y; i < digits_per_snip; i += ROW_SIZE) y = std::min(y, S.data[i]);

  // выровнять по ближайшей снизу четверти
  if(y > 0.75f) y = 0.75f;
  else if (y > 0.50f) y = 0.50f;
  else y = 0.25f;

  // выровнять все вершины по выбранной высоте
  for (size_t i = Y; i < digits_per_snip; i += ROW_SIZE) S.data[i] = y;
  sides_set(R);       // настроить боковые стороны
  rig_display(R);     // записать модифицированый риг в графический буфер
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
  VisibleSnips.clear();
  render_points = 0;

  cfg::DataBase.rigs_loader(MapRigs, From, To);
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


/*
///
/// \brief rdb::snip_update
/// \param s_data
/// \param Point
/// \param dist
///
/// \details Обновление данных снипа, размещенного в VBO.
///
/// Координаты вершин снипов хранятся в виде значений относительно Rig.Original,
/// поэтому перед отправкой данных в VBO координаты вершин пересчитываются.
///
void rdb::snip_update(GLfloat* s_data, const f3d &Point, GLsizeiptr dist)
{

  GLfloat new_data[digits_per_snip] = {0.0f};
  memcpy(new_data, s_data, bytes_per_snip);
  for(size_t n = 0; n < vertices_per_snip; n++)
  {
    new_data[ROW_SIZE * n + X] += Point.x;
    new_data[ROW_SIZE * n + Y] += Point.y;
    new_data[ROW_SIZE * n + Z] += Point.z;
  }
  VBO.data_update(dist, new_data, bytes_per_snip);
}
*/
} //namespace
