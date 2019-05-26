/**
 *
 *
 *
 */

#include "voxel.hpp"

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
/// \brief voxel::box
/// \param V
/// \param l
///
voxel::voxel(const i3d& Or): Origin(Or), born(tr::get_msec())
{
  AllCoords = {
    int3{ 0,    size, size },
    int3{ size, size, size },
    int3{ size, size, 0    },
    int3{ 0,    size, 0    },
    int3{ 0,    0,    size },
    int3{ size, 0,    size },
    int3{ size, 0,    0    },
    int3{ 0,    0,    0    }
  };
  init_arrays();
}


///
/// \brief voxel::init_arrays
///
void voxel::init_arrays(void)
{
  for (auto i = 0; i < SIDES_COUNT; ++i)
  {
    vbo_addr[i]  = -1;
    visible[i] = true;
    tex_id[i]  = {7, 5};
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
    VertTexture[VERT_PER_SIDE * s_id + 0].u = u_sz * tex_id[s_id].u;
    VertTexture[VERT_PER_SIDE * s_id + 0].v = v_sz * tex_id[s_id].v;

    VertTexture[VERT_PER_SIDE * s_id + 1].u = u_sz * tex_id[s_id].u;
    VertTexture[VERT_PER_SIDE * s_id + 1].v = v_sz * tex_id[s_id].v + v_sz;

    VertTexture[VERT_PER_SIDE * s_id + 2].u = u_sz * tex_id[s_id].u + u_sz;
    VertTexture[VERT_PER_SIDE * s_id + 2].v = v_sz * tex_id[s_id].v + v_sz;

    VertTexture[VERT_PER_SIDE * s_id + 3].u = u_sz * tex_id[s_id].u + u_sz;
    VertTexture[VERT_PER_SIDE * s_id + 3].v = v_sz * tex_id[s_id].v;
  }
}


///
/// \brief voxel::side_data
/// \param s
/// \details Заполнение массива стороны данными. Если сторона
/// скрытая, то данные не записываются и возвращается false
///
bool voxel::side_fill_data(u_char side, GLfloat* data, const f3d& P)
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
/// \brief voxel::offset_write
/// \param side_id
/// \param n
///
void voxel::offset_write(u_char side_id, GLsizeiptr n)
{
#ifndef NDEBUG
  if(side_id >= SIDES_COUNT)
  {
    info("voxel::offset_write ERR: side_id >= SIDES_COUNT");
    return;
  }
  if(!visible[side_id]) info("voxel::offset_write ERR: using unvisible side");
#endif

  vbo_addr[side_id] = n;
}


///
/// \brief voxel::side_id_by_offset
/// \return
/// \details По указанному смещению определяет какая сторона там находится
///
u_char voxel::side_id_by_offset(GLsizeiptr dst)
{
  for (u_char side_id = 0; side_id < SIDES_COUNT; ++side_id) {
    if(vbo_addr[side_id] == dst) return side_id;
  }
  return SIDES_COUNT;
}


///
/// \brief voxel::offset_read
/// \param side_id
/// \return offset for side
/// \details Для указанной стороны, если она видимая, то возвращает записаный адрес
/// размещения блока данных в VBO. Если не видимая, то -1.
///
GLsizeiptr voxel::offset_read(u_char side_id)
{
#ifndef NDEBUG
  if(side_id >= SIDES_COUNT) ERR ("voxel::offset_read ERR: side_id >= SIDES_COUNT");
#endif
  if(visible[side_id]) return vbo_addr[side_id];
  return -1;
}


///
/// \brief voxel::offset_replace
/// \param old_n
/// \param new_n
///
void voxel::offset_replace(GLsizeiptr old_n, GLsizeiptr new_n)
{
  u_char side_id;
  for (side_id = 0; side_id < SIDES_COUNT; ++side_id)
  {
    if(vbo_addr[side_id] == old_n)
    {
      #ifndef NDEBUG
        if(!visible[side_id]) info("voxel::offset_replace for unvisible side.");
      #endif
      vbo_addr[side_id] = new_n;
      return;
    }
  }

#ifndef NDEBUG
  info("voxel::offset_replace ERR - not found offset " + std::to_string(new_n) + "\n");
#endif
}


///
/// \brief voxel::visible_check
/// \param side_id
/// \param Sp
///
void voxel::visible_check(u_char side_id, voxel* B1)
{
  side_visible_calc(side_id, B1);
  B1->side_visible_calc(opposite(side_id), this);
}


///
/// \brief voxel::side_visible_calc
/// \param side_id
/// \param B1
///
void voxel::side_visible_calc(u_char side_id, voxel* pVox)
{
  if(nullptr != pVox) visible[side_id] = false;
  else visible[side_id] = true;
}


} //tr
