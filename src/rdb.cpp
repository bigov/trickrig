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

/// DEGUG
void show_texture(double* d)
{

  char buf[256];
  std::sprintf(buf,
    "    u      v   \n"
    " --------------\n"
    " %+5.3lf, %+5.3lf\n"
    " %+5.3lf, %+5.3lf\n"
    " %+5.3lf, %+5.3lf\n"
    " %+5.3lf, %+5.3lf\n\n",
      d[12], d[13], d[26], d[27], d[40], d[41], d[54], d[55]);
  std::cout << buf;
}

///
/// \brief rdb::rdb
/// \details КОНСТРУКТОР
///
rdb::rdb(void)
{
  Prog3d.attach_shaders(
    cfg::app_key(SHADER_VERT_SCENE),
    cfg::app_key(SHADER_FRAG_SCENE)
  );
  Prog3d.use();  // слинковать шейдерную рограмму

  // инициализация VAO
  glGenVertexArrays(1, &vao_id);
  glBindVertexArray(vao_id);

  // Число элементов в кубе с длиной стороны = "space_i0_length" элементов:
  u_int n = static_cast<u_int>(pow((lod0_size + lod0_size + 1), 3));

  // Размер данных VBO для размещения снипов:
  VBOdata.allocate(n * bytes_per_snip);

  // настройка положения атрибутов
  VBOdata.attrib(Prog3d.attrib_location_get("position"),
    4, GL_FLOAT, GL_FALSE, tr::bytes_per_vertex, 0 * sizeof(GLfloat));

  VBOdata.attrib(Prog3d.attrib_location_get("color"),
    4, GL_FLOAT, GL_TRUE, tr::bytes_per_vertex, 4 * sizeof(GLfloat));

  VBOdata.attrib(Prog3d.attrib_location_get("normal"),
    4, GL_FLOAT, GL_TRUE, tr::bytes_per_vertex, 8 * sizeof(GLfloat));

  VBOdata.attrib(Prog3d.attrib_location_get("fragment"),
    2, GL_FLOAT, GL_TRUE, tr::bytes_per_vertex, 12 * sizeof(GLfloat));

  //
  // Так как все четырехугольники в снипах индексируются одинаково, то индексный массив
  // заполняем один раз "под завязку" и забываем про него. Число используемых индексов
  // будет всегда соответствовать числу элементов, передаваемых в процедру "glDraw..."
  //
  size_t idx_size = static_cast<size_t>(6 * n * sizeof(GLuint)); // Размер индексного массива
  GLuint *idx_data = new GLuint[idx_size];                       // данные для заполнения
  GLuint idx[6] = {0, 1, 2, 2, 3, 0};                            // шаблон четырехугольника
  GLuint stride = 0;                                             // число описаных вершин
  for(size_t i=0; i < idx_size; i += 6) {                        // заполнить массив для VBO
    for(size_t x = 0; x < 6; x++) idx_data[x + i] = idx[x] + stride;
    stride += 4;                                                 // по 4 вершины на снип
  }
  tr::vbo VBOindex = { GL_ELEMENT_ARRAY_BUFFER };                // Создать индексный буфер
  VBOindex.allocate(static_cast<GLsizei>(idx_size), idx_data);   // и заполнить данными.
  delete[] idx_data;                                             // Удалить исходный массив.

  glBindVertexArray(0);
  Prog3d.unuse();
}


///
/// Рендер кадра
///
void rdb::draw(void)
{
  if (!CachedOffset.empty()) clear_cashed_snips();

  Prog3d.use();   // включить шейдерную программу
  Prog3d.set_uniform("mvp", MatMVP);
  Prog3d.set_uniform("light_direction", glm::vec4(0.2f, 0.9f, 0.5f, 0.0));
  Prog3d.set_uniform("light_bright", glm::vec4(0.5f, 0.5f, 0.5f, 0.0));

  glBindVertexArray(vao_id);
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
  glEnable(GL_DEPTH_TEST);

  // можно все нарисовать за один проход
  glDrawElements(GL_TRIANGLES, static_cast<GLsizei>(render_points), GL_UNSIGNED_INT, nullptr);

  // а можно, если потребуется, то пошагово по ячейкам -
  //GLsizei max = (render_points / indices_per_snip) * vertices_per_snip;
  //for (GLsizei i = 0; i < max; i += vertices_per_snip)
  //{
  //  glDrawElementsBaseVertex(GL_TRIANGLES, indices_per_snip, GL_UNSIGNED_INT,
  //      nullptr, i);
  //}

  // в конец массива добавлен снип подсветки - нарисуем его отдельно
  glDrawElementsBaseVertex(GL_TRIANGLES, indices_per_snip, GL_UNSIGNED_INT,
      nullptr, (render_points / indices_per_snip) * vertices_per_snip);

  glBindVertexArray(0);
  Prog3d.unuse(); // отключить шейдерную программу
}


///
/// \brief rdb::side_make Формирование снипа для боковой стороны рига
/// \param R
///
void rdb::side_make_snip(const std::array<glm::vec4, 4>& v, snip& S)
{
  for(size_t i = 0; i < v.size(); ++i)
  {
    S.data[ROW_SIZE * i + X] = v[i].x;
    S.data[ROW_SIZE * i + Y] = v[i].y;
    S.data[ROW_SIZE * i + Z] = v[i].z;
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
  side_make_snip(V, S);
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
  side_make_snip(V, S);
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
  side_make_snip(V, S);
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
  side_make_snip(V, S);
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

  remove_from_gpu(R1);
  R1->SideZn.clear();    // убрать стенку -Z соседнего блока (если она есть)

  // Если соседний блок без верха или выше, то построить -Z стенку соседнего блока
  if( R1->SideYp.empty() || ((R0y1 < R1y2) && (R0y0 < R1y3)) )
  {  make_Zn(R1->SideYp, R1->SideZn, R0y1, R0y0); }
  else // иначе - построить свою +Z
  {  make_Zp( R0->SideYp, R0->SideZp, R1y3, R1y2 ); }

  place_in_gpu(R1);
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

  remove_from_gpu(R1);
  R1->SideZp.clear();    // убрать +Z соседнего блока (если есть)

  // Если соседний блок без верха или выше, то построить +Z соседнего блока
  if( R1->SideYp.empty() || ((R0y3 < R1y0) && (R0y2 < R1y1)) )
  {  make_Zp( R1->SideYp, R1->SideZp, R0y3, R0y2 ); }
  else // иначе - обновить -Z
  {  make_Zn( R0->SideYp, R0->SideZn, R1y1, R1y0 ); }

  place_in_gpu(R1);
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

  remove_from_gpu(R1);
  R1->SideXn.clear();  // стенку -X соседнего блока убираем

  // Если соседний блок без верха или выше, то строим -X соседнего блока
  if( R1->SideYp.empty() || ((R0y2 < R1y3) && (R0y1 < R1y0)) )
  {  make_Xn( R1->SideYp, R1->SideXn, R0y2, R0y1 ); }
  else // иначе - строим свою +X
  {  make_Xp( R0->SideYp, R0->SideXp, R1y0, R1y3 ); }

  place_in_gpu(R1);
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

  remove_from_gpu(R1);
  R1->SideXp.clear(); // +X соседнего блока убрать

  // Если соседний блок без верха или выше, то построить +X соседнего блока
  if( R1->SideYp.empty() || ((R0y0 < R1y1) && (R0y3 < R1y2)) )
  {  make_Xp( R1->SideYp, R1->SideXp, R0y0, R0y3 ); }
  else // иначе - построить свою -X
  {  make_Xn( R0->SideYp, R0->SideXn, R1y2, R1y1 ); }

  place_in_gpu(R1);
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
  place_in_gpu(&MapRigs[Pt]);
}


///
/// \brief rdb::add_y
/// \details Увеличение размера по координате Y
///
void rdb::add_y(const i3d& Pt)
{
  rig *R = get(Pt);         //1. Выбрать целевой риг
  if(nullptr == R) ERR ("Error: call rdb::add_y for nullptr point");
  if(R->SideYp.empty()) return;
  remove_from_gpu(R); // убрать риг из графического буфера

  snip &S = R->SideYp.front();
  if((S.data[Y + ROW_SIZE * 0] == 1.00f) &&
     (S.data[Y + ROW_SIZE * 1] == 1.00f) &&
     (S.data[Y + ROW_SIZE * 2] == 1.00f) &&
     (S.data[Y + ROW_SIZE * 3] == 1.00f))
  {
    R->SideYp.clear();
    append_rig_Yp({Pt.x, Pt.y + lod, Pt.z});
    place_in_gpu(R);
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
  sides_set(R); // настроить боковые стороны
  place_in_gpu(R);     // записать модифицированый риг в графический буфер
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
    remove_from_gpu(R);
  }

  snip S = {};
  for (size_t i = Y; i < digits_per_snip; i += ROW_SIZE) S.data[i] = 1.f;
  S.texture_set(AppWin.texYp.u, AppWin.texYp.v);
  R->SideYp.clear();
  R->SideYp.push_back(S);
  place_in_gpu(R);     // записать в графический буфер

  // Обновить боковые стороны вокруг удаленного рига
  R = get({P.x + lod, P.y, P.z});
  if(nullptr != R)
  {
    remove_from_gpu(R);  // убрать риг из графического буфера
    make_Xn( R->SideYp, R->SideXn, 0.f, 0.f );
    place_in_gpu(R);     // записать модифицированый риг в графический буфер
  }

  R = get({P.x - lod, P.y, P.z});
  if(nullptr != R)
  {
    remove_from_gpu(R);  // убрать риг из графического буфера
    make_Xp( R->SideYp, R->SideXp, 0.f, 0.f );
    place_in_gpu(R);     // записать модифицированый риг в графический буфер
  }

  R = get({P.x, P.y, P.z + lod});
  if(nullptr != R)
  {
    remove_from_gpu(R);  // убрать риг из графического буфера
    make_Zn( R->SideYp, R->SideZn, 0.f, 0.f );
    place_in_gpu(R);     // записать модифицированый риг в графический буфер
  }

  R = get({P.x, P.y, P.z - lod});
  if(nullptr != R)
  {
    remove_from_gpu(R);  // убрать риг из графического буфера
    make_Zp( R->SideYp, R->SideZp, 0.f, 0.f );
    place_in_gpu(R);     // записать модифицированый риг в графический буфер
  }
}


///
/// \brief rdb::sub_y
/// \details Уменьшение рига по координате Y.
/// Если высота рига не больше ( 0,25*lod ), то этот риг
/// удаляется и рисуется риг на один шаг ниже.
///
void rdb::sub_y(const i3d& Pt)
{
  rig *R = get(Pt);         //1. Выбрать целевой риг
  if(nullptr == R) ERR ("Error: call rdb::sub_y for nullptr point");
  if(R->SideYp.empty()) return;

  remove_from_gpu(R); // убрать риг из графического буфера

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
  place_in_gpu(R);     // записать модифицированый риг в графический буфер
}


///
/// Добавление в графический буфер элементов, расположенных в точке (x, y, z)
///
void rdb::place_in_gpu(rig* R)
{
  if(nullptr == R) return;   // TODO: тут можно подгружать или дебажить
  if(R->in_vbo) return;      // Если данные уже в VBO - ничего не делаем

  f3d Point = {
    static_cast<float>(R->Origin.x) + R->shift[SHIFT_X],
    static_cast<float>(R->Origin.y) + R->shift[SHIFT_Y],
    static_cast<float>(R->Origin.z) + R->shift[SHIFT_Z]  // TODO: еще есть поворот и zoom
  };

  side_place(R->SideXp, Point);
  side_place(R->SideXn, Point);
  side_place(R->SideYp, Point);
  side_place(R->SideYn, Point);
  side_place(R->SideZp, Point);
  side_place(R->SideZn, Point);

  R->in_vbo = true;
}


///
/// \brief rdb::put_in_vbo
/// \param Rig
/// \param Point
///
void rdb::side_place(std::vector<snip>& Side, const f3d& Point)
{
  for(snip& Snip: Side)
  {
    bool data_is_recieved = false;
    while (!data_is_recieved)
    {
      if(CachedOffset.empty()) // Если кэш пустой, то добавляем данные в конец VBO
      {
        Snip.vbo_append(Point, VBOdata);
        render_points += tr::indices_per_snip;  // увеличить число точек рендера
        VisibleSnips[Snip.data_offset] = &Snip; // добавить ссылку
        data_is_recieved = true;
      }
      else // если в кэше есть адреса свободных мест, то используем
      {    // их с контролем, успешно ли был перемещен блок данных
        data_is_recieved = Snip.vbo_update(Point, VBOdata, CachedOffset.front());
        CachedOffset.pop_front();                                    // укоротить кэш
        if(data_is_recieved) VisibleSnips[Snip.data_offset] = &Snip; // добавить ссылку
      }
    }
  }
}


///
/// Подсветка выделенного рига
///
void rdb::highlight(const i3d& sel)
{
  // Вариант 1: изменить цвет снипа/рига чтобы было понятно, что он выделен.
  //return;

  // Вариант 2: дорисовка прозрачной оболочки над(вокруг) выделенного рига

  rig* R = get({sel.x, sel.y, sel.z});
  if(nullptr == R) return;

  auto highlight = R->SideYp.front();
  highlight.texture_set(0, 7); // белая текстура

  for(size_t n = 0; n < vertices_per_snip; n++)
  {
    highlight.data[ROW_SIZE * n + X] += sel.x;
    highlight.data[ROW_SIZE * n + Y] += sel.y + 0.001f;
    highlight.data[ROW_SIZE * n + Z] += sel.z;
    highlight.data[ROW_SIZE * n + A] = 0.4f; // прозрачность
  }

  // Записать данные снипа подсветки в конец буфера данных VBO
  VBOdata.data_append_tmp(tr::bytes_per_snip, highlight.data);
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
void rdb::remove_from_gpu(rig* Rig)
{
  if(nullptr == Rig) return;
  if(!Rig->in_vbo) return;

  side_remove(Rig->SideYp);
  side_remove(Rig->SideYn);
  side_remove(Rig->SideZp);
  side_remove(Rig->SideZn);
  side_remove(Rig->SideXp);
  side_remove(Rig->SideXn);

  Rig->in_vbo = false;
}


///
/// \brief rdb::side_remove
/// \param Side
///
void rdb::side_remove(std::vector<snip>& Side)
{
  if(Side.empty()) return;

  for(auto& Snip: Side)
  {
    VisibleSnips.erase(Snip.data_offset);
    CachedOffset.push_front(Snip.data_offset);
  }
}


///
/// \brief rdb::clear_cashed_snips
/// \details Удаление элементов по адресам с кэше и сжатие данных в VBO
///
/// Если в кэше есть адрес блока из середины VBO, то в него переносим данные
/// из конца VBO и сжимаем буфер на длину одного блока. Если адрес из кэша
/// указывает на крайний блок в конце VBO, то сжимаем буфер сдвигая границу
/// на длину одного блока.
///
/// Не забываем уменьшить число элементов в рендере.
///
void rdb::clear_cashed_snips(void)
{

  // Выбрать самый крайний элемент VBO на границе блока данных
  GLsizeiptr data_src = VBOdata.get_hem();

  if(0 == render_points)
  {
    CachedOffset.clear();   // очистить все, сообщить о проблеме
    VisibleSnips.clear();   // и закончить обработку кэша
    return;
  }

  #ifndef NDEBUG
  if (data_src == 0 ) {      // Если (вдруг!) данных нет, то
    CachedOffset.clear();   // очистить все, сообщить о проблеме
    VisibleSnips.clear();   // и закончить обработку кэша
    render_points = 0;
    info("WARNING: space::clear_cashed_snips got empty data_src\n");
    return;
  }

  /// Граница буфера (VBOdata.get_hem()), если она не равна нулю,
  /// не может быть меньше размера блока данных (bytes_per_snip)
  if(data_src < tr::bytes_per_snip)
    ERR ("BUG!!! space::jam_vbo got error address in VBO");
  #endif

  data_src -= tr::bytes_per_snip; // адрес последнего блока

  /// Если крайний блок не в списке VisibleSnips, то он и не в рендере.
  /// Поэтому просто отбросим его, сдвинув границу буфера VBO. Кэш не
  /// изменяем, так как в контейнере "forward_list" удаление элементов
  /// из середины списка - затратная операция.
  ///
  /// Внимание! Так как после этого где-то в кэше остается невалидный
  /// (за рабочей границей VBO) адрес блока, то при использовании
  /// адресов из кэша надо делать проверку - не "протухли" ли они.
  ///
  if(VisibleSnips.find(data_src) == VisibleSnips.end())
  {
    VBOdata.shrink(tr::bytes_per_snip);    // укоротить VBO данных
    render_points -= tr::indices_per_snip; // уменьшить число точек рендера
    return;                                // и прервать обработку кэша
  }

  GLsizeiptr data_dst = data_src;
  // Извлечь из кэша адрес
  while(data_dst >= data_src)
  {
    if(CachedOffset.empty()) return;
    data_dst = CachedOffset.front();
    CachedOffset.pop_front();

    // Идеальный вариант, когда освободившийся блок оказался крайним в VBO
    if(data_dst == data_src)
    {
      VBOdata.shrink(tr::bytes_per_snip);    // укоротить VBO данных
      render_points -= tr::indices_per_snip; // уменьшить число точек рендера
      return;                                // закончить шаг обработки
    }
  }

  // Самый частый и самый сложный вариант
  try { // Если есть отображаемый data_src и меньший data_dst из кэша, то
    snip *Snip = VisibleSnips.at(data_src);  // найти перемещаемый снип,
    VisibleSnips.erase(data_src);                // удалить его из карты
    Snip->vbo_jam(VBOdata, data_dst);            // переместить данные в VBO
    VisibleSnips[data_dst] = Snip;               // внести ссылку в карту
    render_points -= tr::indices_per_snip;       // Так как данные из хвоста
  } catch(std::exception & e) {
    ERR(e.what());
  } catch(...) {
    ERR("rigs::clear_cashed_snips got error VisibleSnips[data_src]");
  }
}


///
/// \brief Загрузка из базы данных в оперативную память блока пространства
///
/// \details  Формирование в оперативной памяти карты ригов (std::map) для
/// выбраной области пространства. Из этой карты берутся данные снипов,
/// размещаемых в VBO для рендера сцены.
///
void rdb::load_space(int g, const glm::vec3& Position)
{
  i3d P{ Position };
  lod = g; // TODO проверка масштаба на допустимость

  // Загрузка фрагмента карты 8х8х(16x16) раз на xz плоскости
  i3d From {P.x - 64, 0, P.z - 64};
  i3d To {P.x + 64, 1, P.z + 64};

  MapRigs.clear();
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

} //namespace
