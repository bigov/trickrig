#include "box.hpp"

namespace tr
{

///
/// \brief cross
/// \param s
/// \return
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
/// \brief splice::splice
///
splice::splice(void)
{
  for (size_t i = 0; i < SPLICE_SIZE; ++i) data[i] = 0;
}


///
/// \brief splice::operator!=
/// \param Other
/// \return
///
bool splice::operator!= (splice& Other)
{
  for(size_t i = 0; i < SPLICE_SIZE; ++i) if( data[i] == Other.data[i] ) return false;
  return true;
}


///
/// \brief box::box
/// \param V
/// \param l
///
box::box(uch3 B, uch3 L, void* r): ParentRig(r)
{
  AllCoords = {
    uch3{ B.x              , u_char(B.y + L.y), u_char(B.z + L.z) },
    uch3{ u_char(B.x + L.x), u_char(B.y + L.y), u_char(B.z + L.z) },
    uch3{ u_char(B.x + L.x), u_char(B.y + L.y), B.z               },
    uch3{ B.x              , u_char(B.y + L.y), B.z               },
    uch3{ B.x              , B.y              , u_char(B.z + L.z) },
    uch3{ u_char(B.x + L.x), B.y              , u_char(B.z + L.z) },
    uch3{ u_char(B.x + L.x), B.y              , B.z               },
    uch3{ B.x              , B.y              , B.z               }
  };
  init_arrays();
}


///
/// \brief box::box
/// \param V
/// \details Конструктор бокса по готовому набору из 8 вершин
///
box::box(const std::array<uch3, VERT_PER_BOX>& Arr, void* r): ParentRig(r)
{
  AllCoords = Arr;
  init_arrays();
}


///
/// \brief box::init_arrays
///
void box::init_arrays(void)
{
  //offset   = {0, 0, 0, 0, 0, 0};
  for (auto i = 0; i < SIDES_COUNT; ++i)
  {
    offset[i]  = 0;
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

  for(u_char s_id = 0; s_id < SIDES_COUNT; ++s_id)
  {
    splice_calc(s_id);

    for(u_char v_i = 0; v_i < VERT_PER_SIDE; ++v_i)
    {
      Texture2d[VERT_PER_SIDE * s_id + v_i].u =
          u_sz * tex_id[s_id].u + u_sz * Splice[s_id].data[2*v_i]/255.f;
      Texture2d[VERT_PER_SIDE * s_id + v_i].v =
          v_sz * tex_id[s_id].v + v_sz * (255-Splice[s_id].data[2*v_i+1])/255.f;
    }
  }
}


///
/// \brief box::side_data
/// \param s
/// \details Заполнение массива стороны данными
///
bool box::side_fill_data(u_char side, std::array<GLfloat, digits_per_snip>& data)
{
  if(!visible[side]) return false;

  size_t i = 0;
  for(size_t n = 0; n < vertices_per_snip; ++n)
  {
    data[i++] = AllCoords[(CursorCoord[side][n])].x/255.f;
    data[i++] = AllCoords[(CursorCoord[side][n])].y/255.f;
    data[i++] = AllCoords[(CursorCoord[side][n])].z/255.f;

    data[i++] = AllColors[(CursorColor[side][n])].r;
    data[i++] = AllColors[(CursorColor[side][n])].g;
    data[i++] = AllColors[(CursorColor[side][n])].b;
    data[i++] = AllColors[(CursorColor[side][n])].a;

    data[i++] = AllNormals[(CursorNormal[side][n])].nx;
    data[i++] = AllNormals[(CursorNormal[side][n])].ny;
    data[i++] = AllNormals[(CursorNormal[side][n])].nz;

    data[i++] = Texture2d[VERT_PER_SIDE * side + n].u;
    data[i++] = Texture2d[VERT_PER_SIDE * side + n].v;
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
  if(side_id >= SIDES_COUNT) info("box::offset_write ERR: side_id >= SIDES_COUNT");
  if(!visible[side_id]) info("box::offset_write ERR: using unvisible side");
#endif

  offset[side_id] = n;
}


///
/// \brief box::side_id_offset
/// \return
/// \details По указанному смещению определяет какая сторона там находится
///
u_char box::side_id_offset(GLsizeiptr p)
{
  for (u_char i = 0; i < SIDES_COUNT; ++i) {
    if(offset[i] == p) return i;
  }
  return SIDES_COUNT;
}


///
/// \brief box::offset_read
/// \param side_id
/// \return offset for side
/// \details Для указанной стороны возвращает записаный адрес размещения блока данных в VBO
///
GLsizeiptr box::offset_read(u_char side_id)
{
#ifndef NDEBUG
  if(side_id >= SIDES_COUNT) info("box::offset_read ERR: side_id >= SIDES_COUNT");
#endif
  if(visible[side_id]) return offset[side_id];
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
    if(offset[side_id] == old_n)
    {
      offset[side_id] = new_n;
      return;
    }
  }
#ifndef NDEBUG
  if(!visible[side_id]) info("box::offset_replace for unvisible side.");
  info("box::offset_replace ERR.");
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
/// \brief box::splice_get
/// \param side_id
/// \return
///
splice box::splice_get(u_char side_id)
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
  visible_recheck(side_id, B1);
  B1.visible_recheck(opposite(side_id), *this);
}


///
/// \brief box::visible_recheck
/// \param side_id
/// \param B1
///
void box::visible_recheck(u_char side_id, box& B1)
{
  splice Sp1 = B1.splice_get(opposite(side_id));
  visible[side_id] = (Splice[side_id] != Sp1);
  if(!Sp1.on) visible[side_id] = true;
  if(!Splice[side_id].on) visible[side_id] = true;
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
  a_uch4 id = { CursorCoord[s][1], CursorCoord[s][0], CursorCoord[s][3], CursorCoord[s][2] };

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
  // Индексы вершин, используемых для расчета сплайса
  a_uch4 id = CursorCoord[s];

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
  a_uch4 id = { CursorCoord[s][3], CursorCoord[s][2], CursorCoord[s][1], CursorCoord[s][0] };

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
  a_uch4 id = CursorCoord[s];

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
  a_uch4 id = { CursorCoord[s][1], CursorCoord[s][0], CursorCoord[s][3], CursorCoord[s][2] };

  Splice[s].on = true;
  for(u_char i = 0; i < VERT_PER_SIDE; ++i)
  {
    if(AllCoords[id[i]].z != 0) Splice[s].on = false;
    Splice[s].data[i*2]   = AllCoords[id[i]].x;
    Splice[s].data[i*2+1] = AllCoords[id[i]].y;
  }
}

} //tr
