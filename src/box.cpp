#include "box.hpp"
#include "rig.hpp"

namespace tr
{

///
/// \brief cross
/// \param s
/// \return номер стороны, противоположной указанной в параметре
///
u_char opposite(u_char s)
{
  switch (s) {
    case SIDE_XP:
      return SIDE_XN;
      break;
    case SIDE_XN:
      return SIDE_XP;
      break;
    case SIDE_YP:
      return SIDE_YN;
      break;
    case SIDE_YN:
      return SIDE_YP;
      break;
    case SIDE_ZP:
      return SIDE_ZN;
      break;
    case SIDE_ZN:
      return SIDE_ZP;
      break;
    default:
#ifndef NDEBUG
      info("no opposite for side = " + std::to_string(s));
#endif
      return UCHAR_MAX;
  }
}


///
/// \brief opposite_idx
/// \param side_id
/// \param idx
/// \return номер вершины, противовположной указанной для указанной стороны
///
u_char opposite_idx(u_char side_id, u_char i)
{
  switch (side_id)
  {
    case SIDE_XP:
      if(i == 2) return 3;
      if(i == 1) return 0;
      if(i == 5) return 4;
      if(i == 6) return 7;
      break;
    case SIDE_XN:
      if(i == 0) return 1;
      if(i == 3) return 2;
      if(i == 7) return 6;
      if(i == 4) return 5;
      break;
    case SIDE_YP:
      if(i == 0) return 4;
      if(i == 1) return 5;
      if(i == 2) return 6;
      if(i == 3) return 7;
      break;
    case SIDE_YN:
      if(i == 7) return 3;
      if(i == 6) return 2;
      if(i == 5) return 1;
      if(i == 4) return 0;
      break;
    case SIDE_ZP:
      if(i == 1) return 2;
      if(i == 0) return 3;
      if(i == 4) return 7;
      if(i == 5) return 6;
      break;
    case SIDE_ZN:
      if(i == 3) return 0;
      if(i == 2) return 1;
      if(i == 6) return 5;
      if(i == 7) return 4;
      break;
    default:
#ifndef NDEBUG
      info("no opposite_idx for side = " + std::to_string(side_id));
#endif
  }
  return UCHAR_MAX;
}


///
/// \brief splice::operator!=
/// \param Other
/// \return
///
bool splice::operator!= (splice& Other)
{
  if(!on) return true;
  if(!Other.on) return true;

  u_char buf = 0;
  for(size_t i = 0; i < SPLICE_SIZE; ++i)
  {
    if( data[i] == Other.data[i] ) buf += 1;
  }
  if(buf == SPLICE_SIZE) return false;

  return true;
}


///
/// \brief box::box
/// \param V
/// \param l
///
box::box(rig* r, u_char L ): ParentRig(r)
{
  AllCoords = {
    uch3{ 0, L, L },
    uch3{ L, L, L },
    uch3{ L, L, 0 },
    uch3{ 0, L, 0 },
    uch3{ 0, 0, L },
    uch3{ L, 0, L },
    uch3{ L, 0, 0 },
    uch3{ 0, 0, 0 }
  };
  init_arrays();
}


///
/// \brief box::init_arrays
///
void box::init_arrays(void)
{
  for (auto i = 0; i < SIDES_COUNT; ++i)
  {
    vbo_addr[i]  = -1;
    visible[i] = true;
    tex_id[i]  = {0, 0};
  }

  AllColors = { color{1.f, 1.f, 1.f, 1.f} }; // цвет по-умолчанию для всех вершин

  AllNormals = { // Направление нормалей по сторонам
    { 1.0f, 0.0f, 0.0f }, //xp
    {-1.0f, 0.0f, 0.0f }, //xn
    { 0.0f, 1.0f, 0.0f }, //yp
    { 0.0f,-1.0f, 0.0f }, //yn
    { 0.0f, 0.0f, 1.0f }, //zp
    { 0.0f, 0.0f,-1.0f }  //zn
  };

  IdxCoord = { // индексы координат из массива вершин, для построения сторон
    a_uch4{ 2, 1, 5, 6 }, //x+
    a_uch4{ 0, 3, 7, 4 }, //x-
    a_uch4{ 0, 1, 2, 3 }, //y+
    a_uch4{ 7, 6, 5, 4 }, //y-
    a_uch4{ 1, 0, 4, 5 }, //z+
    a_uch4{ 3, 2, 6, 7 }  //z-
  };

  IdxColor = { // индексы для цветов всех вершин всех 6 сторон
    a_uch4{ 0, 0, 0, 0 }, //x+
    a_uch4{ 0, 0, 0, 0 }, //x-
    a_uch4{ 0, 0, 0, 0 }, //y+
    a_uch4{ 0, 0, 0, 0 }, //y-
    a_uch4{ 0, 0, 0, 0 }, //z+
    a_uch4{ 0, 0, 0, 0 }, //z-
  };

  IdxNormal = { // индексы на нормали вершин
    a_uch4{ 0, 0, 0, 0 }, //x+
    a_uch4{ 1, 1, 1, 1 }, //x-
    a_uch4{ 2, 2, 2, 2 }, //y+
    a_uch4{ 3, 3, 3, 3 }, //y-
    a_uch4{ 4, 4, 4, 4 }, //z+
    a_uch4{ 5, 5, 5, 5 }, //z-
  };

  for(u_char s_id = 0; s_id < SIDES_COUNT; ++s_id)
  {
    splice_calc(s_id);
    texture_calc(s_id);
  }
}


///
/// \brief box::texture_calc
/// \param s_id
///
void box::texture_calc(u_char s_id)
{
  for(u_char v_i = 0; v_i < VERT_PER_SIDE; ++v_i)
  {
    VertTexture[VERT_PER_SIDE * s_id + v_i].u =
        u_sz * tex_id[s_id].u + u_sz * Splice[s_id].data[2*v_i]/255.f;

    VertTexture[VERT_PER_SIDE * s_id + v_i].v =
        v_sz * tex_id[s_id].v + v_sz * (255-Splice[s_id].data[2*v_i+1])/255.f;
  }
}


///
/// \brief box::side_data
/// \param s
/// \details Заполнение массива стороны данными. Если сторона
/// скрытая, то данные не записываются и возвращается false
///
bool box::side_fill_data(u_char side, GLfloat* data, const f3d& P)
{
  if(!visible[side]) return false;

  size_t i = 0;
  for(size_t n = 0; n < vertices_per_quad; ++n)
  {
    data[i++] = AllCoords[(IdxCoord[side][n])].x/255.f + P.x;
    data[i++] = AllCoords[(IdxCoord[side][n])].y/255.f + P.y;
    data[i++] = AllCoords[(IdxCoord[side][n])].z/255.f + P.z;

    data[i++] = AllColors[(IdxColor[side][n])].r;
    data[i++] = AllColors[(IdxColor[side][n])].g;
    data[i++] = AllColors[(IdxColor[side][n])].b;
    data[i++] = AllColors[(IdxColor[side][n])].a;

    data[i++] = AllNormals[(IdxNormal[side][n])].nx;
    data[i++] = AllNormals[(IdxNormal[side][n])].ny;
    data[i++] = AllNormals[(IdxNormal[side][n])].nz;

    data[i++] = VertTexture[VERT_PER_SIDE * side + n].u;
    data[i++] = VertTexture[VERT_PER_SIDE * side + n].v;
  }
  return true;
}


///
/// \brief box::offset_write
/// \param side_id
/// \param n
///
void box::offset_write(u_char side_id, GLsizeiptr n)
{
#ifndef NDEBUG
  if(side_id >= SIDES_COUNT)
  {
    info("box::offset_write ERR: side_id >= SIDES_COUNT");
    return;
  }
  if(!visible[side_id]) info("box::offset_write ERR: using unvisible side");
#endif

  vbo_addr[side_id] = n;
}


///
/// \brief box::side_id_by_offset
/// \return
/// \details По указанному смещению определяет какая сторона там находится
///
u_char box::side_id_by_offset(GLsizeiptr dst)
{
  for (u_char side_id = 0; side_id < SIDES_COUNT; ++side_id) {
    if(vbo_addr[side_id] == dst) return side_id;
  }
  return SIDES_COUNT;
}


///
/// \brief box::offset_read
/// \param side_id
/// \return offset for side
/// \details Для указанной стороны, если она видимая, то возвращает записаный адрес
/// размещения блока данных в VBO. Если не видимая, то -1.
///
GLsizeiptr box::offset_read(u_char side_id)
{
#ifndef NDEBUG
  if(side_id >= SIDES_COUNT) ERR ("box::offset_read ERR: side_id >= SIDES_COUNT");
#endif
  if(visible[side_id]) return vbo_addr[side_id];
  return -1;
}


///
/// \brief box::offset_replace
/// \param old_n
/// \param new_n
///
void box::offset_replace(GLsizeiptr old_n, GLsizeiptr new_n)
{
  u_char side_id;
  for (side_id = 0; side_id < SIDES_COUNT; ++side_id)
  {
    if(vbo_addr[side_id] == old_n)
    {
      #ifndef NDEBUG
        if(!visible[side_id]) info("box::offset_replace for unvisible side.");
      #endif
      vbo_addr[side_id] = new_n;
      return;
    }
  }

#ifndef NDEBUG
  info("box::offset_replace ERR - not found offset " + std::to_string(new_n) + "\n");
#endif
}


///
/// \brief box::splice_get
/// \param side_id
/// \return
///
splice& box::splice_get(u_char side_id)
{
  return Splice[side_id];
}


///
/// \brief box::visible_check
/// \param side_id
/// \param Sp
///
void box::visible_check(u_char side_id, box& B1)
{
  side_visible_calc(side_id, B1);
  B1.side_visible_calc(opposite(side_id), *this);
}


///
/// \brief box::visible_recheck
/// \param side_id
/// \param B1
///
void box::side_visible_calc(u_char side_id, box& B1)
{
  visible[side_id] = ( Splice[side_id] != B1.splice_get(opposite(side_id)) );
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
  a_uch4 id = IdxCoord[s]; // Индексы вершин для расчета сплайса

  Splice[s].on = true;
  for(u_char i = 0; i < VERT_PER_SIDE; ++i)
  {
    if(AllCoords[id[i]].x != UCHAR_MAX)  Splice[s].on = false;
    Splice[s].data[i*2]    = AllCoords[id[i]].y;
    Splice[s].data[i*2+1]  = AllCoords[id[i]].z;
  }
}


///
/// \brief box::splice_side_xn
///
void box::splice_side_xn(void)
{
  u_char s = SIDE_XN;
  // Индексы вершин, используемых для расчета сплайса
  // обратной стороны выбираются в обратном направлении
  a_uch4 id = { IdxCoord[s][1], IdxCoord[s][0], IdxCoord[s][3], IdxCoord[s][2] };

  Splice[s].on = true;
  for(u_char i = 0; i < VERT_PER_SIDE; ++i)
  {
    if(AllCoords[id[i]].x != 0)  Splice[s].on = false;
    Splice[s].data[i*2]   = AllCoords[id[i]].y;
    Splice[s].data[i*2+1] = AllCoords[id[i]].z;
  }
}


///
/// \brief box::splice_side_yp
///
void box::splice_side_yp(void)
{
  u_char s = SIDE_YP;
  a_uch4 id = IdxCoord[s]; // Индексы вершин для расчета сплайса

  Splice[s].on = true;
  for(u_char i = 0; i < VERT_PER_SIDE; ++i)
  {
    if(AllCoords[id[i]].y != UCHAR_MAX)  Splice[s].on = false;
    Splice[s].data[i*2]   = AllCoords[id[i]].x;
    Splice[s].data[i*2+1] = AllCoords[id[i]].z;
  }
}


///
/// \brief box::splice_side_yn
///
void box::splice_side_yn(void)
{
  u_char s = SIDE_YN;
  // Индексы вершин, используемых для расчета сплайса обратной стороны
  // выбираются в обратном направлении
  a_uch4 id = { IdxCoord[s][3], IdxCoord[s][2], IdxCoord[s][1], IdxCoord[s][0] };

  Splice[s].on = true;
  for(u_char i = 0; i < VERT_PER_SIDE; ++i)
  {
    if(AllCoords[id[i]].y != 0) Splice[s].on = false;
    Splice[s].data[i*2]   = AllCoords[id[i]].x;
    Splice[s].data[i*2+1] = AllCoords[id[i]].z;
  }
}


///
/// \brief box::splice_side_zp
///
void box::splice_side_zp(void)
{
  u_char s = SIDE_ZP;
  // Индексы вершин, используемых для расчета сплайса
  a_uch4 id = IdxCoord[s];

  Splice[s].on = true;
  for(u_char i = 0; i < VERT_PER_SIDE; ++i)
  {
    if(AllCoords[id[i]].z != UCHAR_MAX) Splice[s].on = false;
    Splice[s].data[i*2]   = AllCoords[id[i]].x;
    Splice[s].data[i*2+1] = AllCoords[id[i]].y;
  }
}


///
/// \brief box::splice_side_zn
///
void box::splice_side_zn(void)
{
  u_char s = SIDE_ZN;
  // Индексы вершин, используемых для расчета сплайса обратной стороны
  // выбираются в обратном направлении
  a_uch4 id = { IdxCoord[s][1], IdxCoord[s][0], IdxCoord[s][3], IdxCoord[s][2] };

  Splice[s].on = true;
  for(u_char i = 0; i < VERT_PER_SIDE; ++i)
  {
    if(AllCoords[id[i]].z != 0) Splice[s].on = false;
    Splice[s].data[i*2]   = AllCoords[id[i]].x;
    Splice[s].data[i*2+1] = AllCoords[id[i]].y;
  }
}

} //tr
