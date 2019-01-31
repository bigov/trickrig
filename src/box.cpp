#include "box.hpp"

namespace tr
{

///
/// \brief splice::operator !=
/// \param Other
/// \return
///
bool splice::operator!= (splice& Other)
{
  return (!(*this == Other));
}


///
/// \brief splice::operator ==
/// \param Other
/// \return
///
bool splice::operator== (splice& Other)
{
  size_t n = this->size();
  if(Other.size() != n) return false;

  for(size_t i=0; i<n; ++i) if( *((*this)[i]) != *Other[i] ) return false;
  return true;
}


///
/// \brief box::box
/// \param V
/// \param l
///
box::box(f3d B, float lx, float ly, float lz)
{
  AllCoords = {
    f3d{ B.x     , B.y + ly, B.z + lz },
    f3d{ B.x + lx, B.y + ly, B.z + lz },
    f3d{ B.x + lx, B.y + ly, B.z      },
    f3d{ B.x     , B.y + ly, B.z      },
    f3d{ B.x     , B.y     , B.z + lz },
    f3d{ B.x + lx, B.y     , B.z + lz },
    f3d{ B.x + lx, B.y     , B.z      },
    f3d{ B.x     , B.y     , B.z      }
  };
  init_arrays();
}


///
/// \brief box::box
/// \param V
/// \details Конструктор бокса по готовому набору из 8 вершин
///
box::box(const std::array<f3d, 8>& Arr)
{
  AllCoords = Arr;
  init_arrays();
}


///
/// \brief box::init_arrays
///
void box::init_arrays(void)
{
  AllColors = { color{} }; // цвет по-умолчанию для всех вершин

  AllNormals = { // Направление нормалей по сторонам
    { 1.0f, 0.0f, 0.0f }, //xp
    {-1.0f, 0.0f, 0.0f }, //xn
    { 0.0f, 1.0f, 0.0f }, //yp
    { 0.0f,-1.0f, 0.0f }, //yn
    { 0.0f, 0.0f, 1.0f }, //zp
    { 0.0f, 0.0f,-1.0f }  //zn
  };

  AllTextures = { // текстура по-умолчанию для всех 6 сторон
    {0.0f, 0.0f},
    {u_sz, 0.0f},
    {u_sz, u_sz},
    {0.0f, u_sz}
  };

  CursorCoord = { // индексы координат из массива вершин, для построения сторон
    a_uch4{ 2, 1, 5, 6 }, //x+
    a_uch4{ 0, 3, 7, 4 }, //x-
    a_uch4{ 0, 1, 2, 3 }, //y+
    a_uch4{ 7, 6, 5, 4 }, //y-
    a_uch4{ 1, 0, 4, 5 }, //z+
    a_uch4{ 3, 2, 6, 7 }  //z-
  };

  CursorColor = { // указатель на цвета всех вершин всех 6 сторон
    a_uch4{ 0, 0, 0, 0 }, //x+
    a_uch4{ 0, 0, 0, 0 }, //x-
    a_uch4{ 0, 0, 0, 0 }, //y+
    a_uch4{ 0, 0, 0, 0 }, //y-
    a_uch4{ 0, 0, 0, 0 }, //z+
    a_uch4{ 0, 0, 0, 0 }, //z-
  };

  CursorNormal = { // указатель на нормали вершин
    a_uch4{ 0, 0, 0, 0 }, //x+
    a_uch4{ 1, 1, 1, 1 }, //x-
    a_uch4{ 2, 2, 2, 2 }, //y+
    a_uch4{ 3, 3, 3, 3 }, //y-
    a_uch4{ 4, 4, 4, 4 }, //z+
    a_uch4{ 5, 5, 5, 5 }, //z-
  };

  CursorTexture = {
    a_uch4{ 0, 1, 2, 3 }, //x+
    a_uch4{ 0, 1, 2, 3 }, //x-
    a_uch4{ 0, 1, 2, 3 }, //y+
    a_uch4{ 0, 1, 2, 3 }, //y-
    a_uch4{ 0, 1, 2, 3 }, //z+
    a_uch4{ 0, 1, 2, 3 }, //z-
  };

  for(u_char id = 0; id < SIDES_COUNT; ++id) splice_calc(id);
}


///
/// \brief box::side_data
/// \param s
/// \details Заполнение массива стороны данными
///
void box::side_fill_data(u_char side, std::array<GLfloat, digits_per_snip>& data)
{
  for(size_t n = 0; n < vertices_per_snip; n++)
  {
    auto Coord   = AllCoords  [CursorCoord  [side][n]];
    auto Color   = AllColors  [CursorColor  [side][n]];
    auto Normal  = AllNormals [CursorNormal [side][n]];
    auto Texture = AllTextures[CursorTexture[side][n]];

    data[ROW_SIZE * n + X] = Coord.x;
    data[ROW_SIZE * n + Y] = Coord.y;
    data[ROW_SIZE * n + Z] = Coord.x;

    data[ROW_SIZE * n + R] = Color.r;
    data[ROW_SIZE * n + G] = Color.g;
    data[ROW_SIZE * n + B] = Color.b;
    data[ROW_SIZE * n + A] = Color.a;

    data[ROW_SIZE * n + NX] = Normal.nx;
    data[ROW_SIZE * n + NY] = Normal.ny;
    data[ROW_SIZE * n + NZ] = Normal.nz;

    data[ROW_SIZE * n + U] = Texture.u;
    data[ROW_SIZE * n + V] = Texture.v;
  }
}


///
/// \brief box::offset_write
/// \param side_id
/// \param n
///
void box::offset_write(u_char side_id, GLsizeiptr n)
{
#ifndef NDEBUG
  if(side_id >= SIDES_COUNT) info("box::offset_write ERR: side_id >= SIDES_COUNT");
#endif
  offset[side_id] = n;
  visible[side_id] = true;
}


///
/// \brief box::offset_read
/// \param side_id
/// \return offset for side
///
GLsizeiptr box::offset_read(u_char side_id)
{
#ifndef NDEBUG
  if(side_id >= SIDES_COUNT) info("box::offset_read ERR: side_id >= SIDES_COUNT");
#endif
  return offset[side_id];
}


///
/// \brief box::offset_replace
/// \param old_n
/// \param new_n
///
void box::offset_replace(GLsizeiptr old_n, GLsizeiptr new_n)
{
  u_char side_id;
  for (side_id = 0; side_id < SIDES_COUNT; ++side_id) {
    if(offset[side_id] == old_n) offset[side_id] = new_n;
  }
#ifndef NDEBUG
  if(!visible[side_id]) info("box::offset_replace for unvisible side.");
  if(side_id == SIDES_COUNT) info("box::offset_replace ERR.");
#endif
}


///
/// \brief box::side_id_by_offset
/// \param n
/// \return side_id
///
u_char box::side_id_by_offset(GLsizeiptr n)
{
  for (u_char id = 0; id < SIDES_COUNT; ++id) {
    if(offset[id] == n) return id;
  }
  return  SIDES_COUNT;
}


///
/// \brief box::visible
/// \param s
/// \param V1
/// \return Отображать сторону или нет
///
/// \details сторона отображается если сплайсы не совпадют
///
bool box::is_visible(u_char s, splice& V1)
{
  return (Splice[s] != V1);
}


///
/// \brief box::splice_side
/// \param side_id
///
void box::splice_calc(u_char side_id)
{
  switch (side_id) {
    case SIDE_XP:
      splice_side_xp();
      break;
    case SIDE_XN:
      splice_side_xn();
      break;
    case SIDE_YP:
      splice_side_yp();
      break;
    case SIDE_YN:
      splice_side_yn();
      break;
    case SIDE_ZP:
      splice_side_zp();
      break;
    case SIDE_ZN:
      splice_side_zn();
      break;
    default:
      info("Err side_id on " + std::string(__func__));

  }
}

///
/// \brief box::splice_side_xp
///
void box::splice_side_xp(void)
{
  u_char s = SIDE_XP;
  a_uch4 id = CursorCoord[s]; // Индексы вершин для расчета сплайса

  size_t n = id.size();
  Splice[s].clear();
  Splice[s].resize(n);

  bool all_top = true;
  for(u_char i = 0; i < n; ++i)
  {
    if(AllCoords[id[i]].x != 1.0f)  all_top = false;
    Splice[s][i*2]   = &AllCoords[id[i]].y;
    Splice[s][i*2+1] = &AllCoords[id[i]].z;
  }
  if(!all_top) Splice[s].clear();
}


///
/// \brief box::splice_side_xn
///
void box::splice_side_xn(void)
{
  u_char s = SIDE_XN;
  // Индексы вершин, используемых для расчета сплайса
  // обратной стороны выбираются в обратном направлении
  a_uch4 id = { CursorCoord[s][1], CursorCoord[s][0], CursorCoord[s][3], CursorCoord[s][2] };

  size_t n = id.size();
  Splice[s].clear();
  Splice[s].resize(n);

  bool all_top = true;
  for(u_char i = 0; i < n; ++i)
  {
    if(AllCoords[id[i]].x !=-1.0f)  all_top = false;
    Splice[s][i*2]   = &AllCoords[id[i]].y;
    Splice[s][i*2+1] = &AllCoords[id[i]].z;
  }
  if(!all_top) Splice[s].clear();
}


///
/// \brief box::splice_side_yp
///
void box::splice_side_yp(void)
{
  u_char s = SIDE_YP;
  // Индексы вершин, используемых для расчета сплайса
  a_uch4 id = CursorCoord[s];

  size_t n = id.size();
  Splice[s].clear();
  Splice[s].resize(n);

  bool all_top = true;
  for(u_char i = 0; i < n; ++i)
  {
    if(AllCoords[id[i]].y != 1.0f)  all_top = false;
    Splice[s][i*2]   = &AllCoords[id[i]].x;
    Splice[s][i*2+1] = &AllCoords[id[i]].z;
  }
  if(!all_top) Splice[s].clear();
}


///
/// \brief box::splice_side_yn
///
void box::splice_side_yn(void)
{
  u_char s = SIDE_YN;
  // Индексы вершин, используемых для расчета сплайса обратной стороны
  // выбираются в обратном направлении
  a_uch4 id = { CursorCoord[s][1], CursorCoord[s][0], CursorCoord[s][3], CursorCoord[s][2] };

  size_t n = id.size();
  Splice[s].clear();
  Splice[s].resize(n);

  bool all_top = true;
  for(u_char i = 0; i < n; ++i)
  {
    if(AllCoords[id[i]].y !=-1.0f)  all_top = false;
    Splice[s][i*2]   = &AllCoords[id[i]].x;
    Splice[s][i*2+1] = &AllCoords[id[i]].z;
  }
  if(!all_top) Splice[s].clear();
}


///
/// \brief box::splice_side_zp
///
void box::splice_side_zp(void)
{
  u_char s = SIDE_ZP;
  // Индексы вершин, используемых для расчета сплайса
  a_uch4 id = CursorCoord[s];

  size_t n = id.size();
  Splice[s].clear();
  Splice[s].resize(n);

  bool all_top = true;
  for(u_char i = 0; i < n; ++i)
  {
    if(AllCoords[id[i]].z != 1.0f) all_top = false;
    Splice[s][i*2]   = &AllCoords[id[i]].x;
    Splice[s][i*2+1] = &AllCoords[id[i]].y;
  }
  if(!all_top) Splice[s].clear();
}


///
/// \brief box::splice_side_zn
///
void box::splice_side_zn(void)
{
  u_char s = SIDE_ZN;
  // Индексы вершин, используемых для расчета сплайса обратной стороны
  // выбираются в обратном направлении
  a_uch4 id = { CursorCoord[s][1], CursorCoord[s][0], CursorCoord[s][3], CursorCoord[s][2] };

  size_t n = id.size();
  Splice[s].clear();
  Splice[s].resize(n);

  bool all_top = true;
  for(u_char i = 0; i < n; ++i)
  {
    if(AllCoords[id[i]].z !=-1.0f) all_top = false;
    Splice[s][i*2]   = &AllCoords[id[i]].x;
    Splice[s][i*2+1] = &AllCoords[id[i]].y;
  }
  if(!all_top) Splice[s].clear();
}

} //tr
