/*
 *
 * file: rdb.cpp
 *
 * Управление элементами 3D пространства
 *
 */

#include "rdb.hpp"
#include "config.hpp"

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
/// \brief normal_X
/// \param V
/// \return
///
bool rdb::normal_on_x(const std::array<glm::vec4, 4>& V)
{
  if((V[0].x == V[1].x) && (V[0].x == V[2].x) && (V[0].x == V[3].x)) return true;

  if(
      (V[0].z == V[3].z) && (V[2].z == V[1].z) &&
      (V[1].y == V[0].y) && (V[3].y == V[2].y)
    ) return true;

  return false;
}


///
/// \brief normal_Y
/// \param V
/// \return
///
bool rdb::normal_on_y(const std::array<glm::vec4, 4>& V)
{
  if((V[0].y == V[1].y) && (V[0].y == V[2].y) && (V[0].y == V[3].y)) return true;

  if(
      (V[0].x == V[3].x) && (V[2].x == V[1].x) &&
      (V[1].z == V[0].z) && (V[3].z == V[2].z)
    ) return true;

  return false;
}


///
/// \brief normal_Z
/// \param V
/// \return
///
bool rdb::normal_on_z(const std::array<glm::vec4, 4>& V)
{
  if((V[0].z == V[1].z) && (V[0].z == V[2].z) && (V[0].z == V[3].z)) return true;

  if(
      (V[0].x == V[3].x) && (V[2].x == V[1].x) &&
      (V[1].y == V[0].y) && (V[3].y == V[2].y)
    ) return true;

  return false;
}


///
/// \brief rdb::snip_analise
/// \param S
/// \details Анализ положения снипа в пространстве и установка значений
/// необходимых для вычислений параметров
///
void rdb::snip_analyze(snip_ext& S)
{
  glm::vec4 V0 = S.vertex_coord(0);
  glm::vec4 V1 = S.vertex_coord(1);
  glm::vec4 V2 = S.vertex_coord(2);
  glm::vec4 V3 = S.vertex_coord(3);
  glm::vec4 Normal = S.vertex_normal(0);
  float foo = 0.f;

  if( normal_on_y({V0, V1, V2, V3}) )
  {
    if((modff(V0.y, &foo) == 0) &&
       (modff(V1.y, &foo) == 0) &&
       (modff(V2.y, &foo) == 0) &&
       (modff(V3.y, &foo) == 0)) S.top = true;

    if(Normal.y > 0)
    {
      S.lay = LAY_YP;
      S.Origin = V3;
      if(S.top) S.Origin.y -= lod;
    }
    else
    {
      S.lay = LAY_YN;
      S.Origin = V0;
    }
  }

  if( normal_on_x({V0, V1, V2, V3}) )
  {
    if((modff(V0.x, &foo) == 0) &&
       (modff(V1.x, &foo) == 0) &&
       (modff(V2.x, &foo) == 0) &&
       (modff(V3.x, &foo) == 0)) S.top = true;

    if(Normal.x > 0)
    {
      S.lay = LAY_XP;
      S.Origin = V3;
      if(S.top) S.Origin.x -= lod;
    }
    else
    {
      S.lay = LAY_XN;
      S.Origin = V2;
    }
  }

  if( normal_on_z({V0, V1, V2, V3}) )
  {
    if((modff(V0.z, &foo) == 0) &&
       (modff(V1.z, &foo) == 0) &&
       (modff(V2.z, &foo) == 0) &&
       (modff(V3.z, &foo) == 0)) S.top = true;

    if(Normal.z > 0)
    {
      S.lay = LAY_ZP;
      S.Origin = V2;
      if(S.top) S.Origin.z -= lod;
    }
    else
    {
      S.lay = LAY_ZN;
      S.Origin = V3;
    }
  }

  S.Origin.x = floor(S.Origin.x);
  S.Origin.y = floor(S.Origin.y);
  S.Origin.z = floor(S.Origin.z);
}


void add_yn(i3d&) { info("add YP"); }
void add_xn(i3d&) { info("add XN"); }
void add_xp(i3d&) { info("add XP"); }
void add_zn(i3d&) { info("add ZN"); }
void add_zp(i3d&) { info("add ZP"); }

void sub_yn(i3d&) { info("sub YP"); }
void sub_xn(i3d&) { info("sub XN"); }
void sub_xp(i3d&) { info("sub XP"); }
void sub_zn(i3d&) { info("sub ZN"); }
void sub_zp(i3d&) { info("sub ZP"); }


///
/// \brief rdb::increase
/// \param i
///
/// \details Добавление объема к указанной стороне
///
void rdb::increase(unsigned int i)
{
  GLsizeiptr offset = (i/vertices_per_snip) * bytes_per_snip; // + bytes_per_vertex;
  snip_ext S{};

  VBO->data_get(offset, bytes_per_snip, S.data); // считать из VBO данные снипа
//DEBUG
return;

  snip_analyze(S);
  if(S.lay == LAY_YP) add_yp(S.Origin);
  //if(S.lay == LAY_YN) add_yn(S.Origin);
  //if(S.lay == LAY_XP) add_xp(S.Origin);
  //if(S.lay == LAY_XN) add_xn(S.Origin);
  //if(S.lay == LAY_ZP) add_zp(S.Origin);
  //if(S.lay == LAY_ZN) add_zn(S.Origin);
}


///
/// \brief rdb::decrease
/// \param i
///
/// \details Уменьшение объема с указанной стороны
///
void rdb::decrease(unsigned int i)
{
  GLsizeiptr offset = (i/vertices_per_snip) * bytes_per_snip; // + bytes_per_vertex;
  snip_ext S{};
  VBO->data_get(offset, bytes_per_snip, S.data); // считать из VBO данные снипа
  snip_analyze(S);                               //
  if(S.lay == LAY_YP) sub_yp(S.Origin);
  if(S.lay == LAY_YN) sub_yn(S.Origin);
  if(S.lay == LAY_XP) sub_xp(S.Origin);
  if(S.lay == LAY_XN) sub_xn(S.Origin);
  if(S.lay == LAY_ZP) sub_zp(S.Origin);
  if(S.lay == LAY_ZN) sub_zn(S.Origin);
}


///
/// Добавление в графический буфер элементов, расположенных в точке (x, y, z)
///
void rdb::rig_place(rig* R)
{
  if(nullptr == R) return;   // TODO: тут можно подгружать или дебажить
  if(R->in_vbo) return;      // Если данные уже в VBO - ничего не делаем

  f3d Point = {
    static_cast<float>(R->Origin.x) + R->shift[SHIFT_X],
    static_cast<float>(R->Origin.y) + R->shift[SHIFT_Y],
    static_cast<float>(R->Origin.z) + R->shift[SHIFT_Z]  // TODO: еще есть поворот и zoom
  };

  side_vbo_append(R->SideXp, Point);
  side_vbo_append(R->SideXn, Point);
  side_vbo_append(R->SideYp, Point);
  side_vbo_append(R->SideYn, Point);
  side_vbo_append(R->SideZp, Point);
  side_vbo_append(R->SideZn, Point);

  R->in_vbo = true;
}


///
/// \brief rdb::place_snip
/// \param Side
/// \param Point
///
/// \brief
/// Добавляет снип в конец VBO и записывает адрес смещения блока данных.
///
/// \details
/// Координаты вершин снипов хранятся в нормализованом виде, поэтому перед
/// отправкой в VBO все данные снипа копируются во временный кэш, где
/// координаты вершин пересчитываются с учетом координат (TODO: сдвига и
/// поворота рига-контейнера), после чего данные записываются в VBO.
///
void rdb::side_vbo_append(std::vector<snip>& Side, const f3d& Point)
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
/// \details убрать риг из рендера
///
/// Индексы размещенных в VBO данных, которые при перемещении камеры вышли
/// за границу отображения, запоминаются в кэше, чтобы на их место
/// записать данные вершин, которые вошли в поле зрения с другой стороны.
///
void rdb::rig_remove(rig* Rig)
{
  if(nullptr == Rig) return;
  if(!Rig->in_vbo) return;

  side_vbo_remove(Rig->SideYp);
  side_vbo_remove(Rig->SideYn);
  side_vbo_remove(Rig->SideZp);
  side_vbo_remove(Rig->SideZn);
  side_vbo_remove(Rig->SideXp);
  side_vbo_remove(Rig->SideXn);

  Rig->in_vbo = false;
}


///
/// \brief side_remove
/// \param Side
///
void rdb::side_vbo_remove(std::vector<snip>& Side)
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

  rig_remove(R1);
  R1->SideZn.clear();    // убрать стенку -Z соседнего блока (если она есть)

  // Если соседний блок без верха или выше, то построить -Z стенку соседнего блока
  if( R1->SideYp.empty() || ((R0y1 < R1y2) && (R0y0 < R1y3)) )
  {  make_Zn(R1->SideYp, R1->SideZn, R0y1, R0y0); }
  else // иначе - построить свою +Z
  {  make_Zp( R0->SideYp, R0->SideZp, R1y3, R1y2 ); }

  rig_place(R1);
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

  rig_remove(R1);
  R1->SideZp.clear();    // убрать +Z соседнего блока (если есть)

  // Если соседний блок без верха или выше, то построить +Z соседнего блока
  if( R1->SideYp.empty() || ((R0y3 < R1y0) && (R0y2 < R1y1)) )
  {  make_Zp( R1->SideYp, R1->SideZp, R0y3, R0y2 ); }
  else // иначе - обновить -Z
  {  make_Zn( R0->SideYp, R0->SideZn, R1y1, R1y0 ); }

  rig_place(R1);
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

  rig_remove(R1);
  R1->SideXn.clear();  // стенку -X соседнего блока убираем

  // Если соседний блок без верха или выше, то строим -X соседнего блока
  if( R1->SideYp.empty() || ((R0y2 < R1y3) && (R0y1 < R1y0)) )
  {  make_Xn( R1->SideYp, R1->SideXn, R0y2, R0y1 ); }
  else // иначе - строим свою +X
  {  make_Xp( R0->SideYp, R0->SideXp, R1y0, R1y3 ); }

  rig_place(R1);
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

  rig_remove(R1);
  R1->SideXp.clear(); // +X соседнего блока убрать

  // Если соседний блок без верха или выше, то построить +X соседнего блока
  if( R1->SideYp.empty() || ((R0y0 < R1y1) && (R0y3 < R1y2)) )
  {  make_Xp( R1->SideYp, R1->SideXp, R0y0, R0y3 ); }
  else // иначе - построить свою -X
  {  make_Xn( R0->SideYp, R0->SideXn, R1y2, R1y1 ); }

  rig_place(R1);
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

  // +Z
  R1 = get({R0->Origin.x, R0->Origin.y, R0->Origin.z + lod});
  if(nullptr != R1)
  {
    if((R0y1 < R1y2) && (R0y0 > R1y3)) R0y1 = R1y2;
    if((R0y0 < R1y3) && (R0y1 > R1y2)) R0y0 = R1y3;
  }
  // -Z
  R1 = get({R0->Origin.x, R0->Origin.y, R0->Origin.z - lod});
  if(nullptr != R1)
  {
    if((R0y3 < R1y0) && (R0y2 > R1y1)) R0y3 = R1y0;
    if((R0y2 < R1y1) && (R0y3 > R1y0)) R0y2 = R1y1;
  }
  // +X
  R1 = get({R0->Origin.x + lod, R0->Origin.y, R0->Origin.z});
  if(nullptr != R1)
  {
    if((R0y2 < R1y3) && (R0y1 > R1y0)) R0y2 = R1y3;
    if((R0y1 < R1y0) && (R0y2 > R1y3)) R0y1 = R1y0;
  }
  // -X
  R1 = get({R0->Origin.x - lod, R0->Origin.y, R0->Origin.z});
  if(nullptr != R1)
  {
    if((R0y0 < R1y1) && (R0y3 > R1y2)) R0y0 = R1y1;
    if((R0y3 < R1y2) && (R0y0 > R1y1)) R0y3 = R1y2;
  }

  set_Zp( R0, get({R0->Origin.x, R0->Origin.y, R0->Origin.z + lod}) );
  set_Zn( R0, get({R0->Origin.x, R0->Origin.y, R0->Origin.z - lod}) );
  set_Xp( R0, get({R0->Origin.x + lod, R0->Origin.y, R0->Origin.z}) );
  set_Xn( R0, get({R0->Origin.x - lod, R0->Origin.y, R0->Origin.z}) );
}


///
/// \brief rdb::append_rig_Yp
/// \param Pt
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
  rig_place(&MapRigs[Pt]);
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
  rig_remove(R); // убрать риг из графического буфера

  snip &S = R->SideYp.front();
  if((S.data[Y + ROW_SIZE * 0] == 1.00f) &&
     (S.data[Y + ROW_SIZE * 1] == 1.00f) &&
     (S.data[Y + ROW_SIZE * 2] == 1.00f) &&
     (S.data[Y + ROW_SIZE * 3] == 1.00f))
  {
    R->SideYp.clear();
    append_rig_Yp({Pt.x, Pt.y + lod, Pt.z});
    rig_place(R);
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
  rig_place(R);   // записать модифицированый риг в графический буфер
}


///
/// \brief rdb::remove_rig_Yp
/// \param Pt
/// \details Удаляет риг в указанной точке пространства и обновляет
/// боковые стороны ригов примыкающих к данной точке.
///
void rdb::remove_rig_Yp(const i3d& P)
{
  MapRigs.erase(P);             // Удалить риг в точке P пространства 3D

  i3d Psub {P.x, P.y-lod, P.z}; // Координаты точки на шаг ниже
  rig* R = get(Psub);
  if(nullptr == R)              // Если снизу рига нет, то следует его создать
  {
    MapRigs.emplace(std::pair(Psub, rig{}));
    MapRigs[Psub].Origin = Psub;
    R = get(Psub);
  }
  else {
    rig_remove(R);
  }

  snip S = {};
  for (size_t i = Y; i < digits_per_snip; i += ROW_SIZE) S.data[i] = 1.f;
  S.texture_set(AppWin.texYp.u, AppWin.texYp.v);
  R->SideYp.clear();
  R->SideYp.push_back(S);
  rig_place(R);     // записать в графический буфер

  // Обновить боковые стороны вокруг удаленного рига
  R = get({P.x + lod, P.y, P.z});
  if(nullptr != R)
  {
    rig_remove(R);  // убрать риг из графического буфера
    make_Xn( R->SideYp, R->SideXn, 0.f, 0.f );
    rig_place(R);     // записать модифицированый риг в графический буфер
  }

  R = get({P.x - lod, P.y, P.z});
  if(nullptr != R)
  {
    rig_remove(R);  // убрать риг из графического буфера
    make_Xp( R->SideYp, R->SideXp, 0.f, 0.f );
    rig_place(R);     // записать модифицированый риг в графический буфер
  }

  R = get({P.x, P.y, P.z + lod});
  if(nullptr != R)
  {
    rig_remove(R);  // убрать риг из графического буфера
    make_Zn( R->SideYp, R->SideZn, 0.f, 0.f );
    rig_place(R);     // записать модифицированый риг в графический буфер
  }

  R = get({P.x, P.y, P.z - lod});
  if(nullptr != R)
  {
    rig_remove(R);  // убрать риг из графического буфера
    make_Zp( R->SideYp, R->SideZp, 0.f, 0.f );
    rig_place(R);     // записать модифицированый риг в графический буфер
  }
}


///
/// \brief rdb::sub_y
/// \details Уменьшение рига по координате Y.
/// Если высота рига не больше ( 0,25*lod ), то этот риг
/// удаляется и рисуется риг на один шаг ниже.
///
void rdb::sub_yp(const i3d& Pt)
{
  rig *R = get(Pt);         //1. Выбрать целевой риг
  if(nullptr == R) ERR ("Error: call rdb::sub_y for nullptr point");
  if(R->SideYp.empty()) return;

  rig_remove(R); // убрать риг из графического буфера

  snip &S = R->SideYp.front();

  if((S.data[Y + ROW_SIZE * 0] <= 0.25f) ||
     (S.data[Y + ROW_SIZE * 1] <= 0.25f) ||
     (S.data[Y + ROW_SIZE * 2] <= 0.25f) ||
     (S.data[Y + ROW_SIZE * 3] <= 0.25f))
  {                                         // Если одна из вершин данного рига
    remove_rig_Yp(Pt);                      // расположена ниже у=0.25, то этот
    return;                                 // риг полностью удаляется
  }

  float y = 1.f; // найти вершину с минимальным значением Y
  for (size_t i = Y; i < digits_per_snip; i += ROW_SIZE) y = std::min(y, S.data[i]);

  // выровнять по ближайшей снизу четверти
  if(y > 0.75f) y = 0.75f;
  else if (y > 0.50f) y = 0.50f;
  else y = 0.25f;

  // выровнять все вершины по выбранной высоте
  for (size_t i = Y; i < digits_per_snip; i += ROW_SIZE) S.data[i] = y;
  sides_set(R); // настроить боковые стороны
  rig_place(R);     // записать модифицированый риг в графический буфер
}


///
/// Подсветка выделенного рига
///
void rdb::highlight(const i3d&)
{
  return;

  // Вариант 1: изменить цвет снипа/рига чтобы было понятно, что он выделен.
  //return;

  // Вариант 2: выделение границ выделенного снипа
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
