/**
 *
 *
 *
 */

#include "voxel.hpp"

namespace tr
{
///
/// \brief voxel::box
/// \param Origin point
/// \param Side length
///
voxel::voxel(const i3d& Or, int sz)
: Origin(Or), side_len(sz), born(tr::get_msec())
{
  init_data();
}


///
/// \brief voxel::side_color_set
/// \param side
/// \param C
///
void voxel::side_color_set(u_int side, color C)
{
  size_t i = side * digits_per_side;
  for(u_int v = 0; v < vertices_per_side; ++v)
  {
    data[i + R] = C.r;
    data[i + G] = C.g;
    data[i + B] = C.b;
    data[i + A] = C.a;
    i += digits_per_vertex;
  }
}


///
/// \brief voxel::side_texture_set
/// \param side
/// \param texture
///
void voxel::side_texture_set(u_int side)
{
  uch2 texture = tex_id[side];

  size_t i =  side * digits_per_side;
  data[i + U] = u_sz * texture.u;
  data[i + V] = v_sz * texture.v;

  i += digits_per_vertex;
  data[i + U] = u_sz * texture.u + u_sz;
  data[i + V] = v_sz * texture.v;

  i += digits_per_vertex;
  data[i + U] = u_sz * texture.u + u_sz;
  data[i + V] = v_sz * texture.v + v_sz;

  i += digits_per_vertex;
  data[i + U] = u_sz * texture.u;
  data[i + V] = v_sz * texture.v + v_sz;
}


///
/// \brief voxel::side_normals_set
/// \param side
///
void voxel::side_normals_set(u_int side)
{
  GLfloat nx = 0.f, ny = 0.f, nz = 0.f;

  switch (side) {
    case SIDE_XP:
      nx = 1.f;
      break;
    case SIDE_XN:
      nx =-1.f;
      break;
    case SIDE_YP:
      ny = 1.f;
      break;
    case SIDE_YN:
      ny =-1.f;
      break;
    case SIDE_ZP:
      nz = 1.f;
      break;
    case SIDE_ZN:
      nz =-1.f;
  }

  size_t i =  side * digits_per_side;
  for(u_int v = 0; v < vertices_per_side; ++v)
  {
    data[i + NX] = nx;
    data[i + NY] = ny;
    data[i + NZ] = nz;
    i += digits_per_vertex;
  }
}


///
/// \brief voxel::side_position_set
/// \param side
///
void voxel::side_position_set(u_int side)
{
  // относительные координаты всех вершин вокселя
  f3d P[8] = {{side_len, side_len, 0}, {side_len, side_len, side_len}, {side_len, 0, side_len},
              {side_len, 0, 0}, {0, side_len, side_len}, {0, side_len, 0}, {0, 0, 0}, {0, 0, side_len}};

  f3d sh[4]; // смещения 4-х образующих вершин выбранной стороны вокселя
  switch (side) {
    case SIDE_XP:
      sh[0] = P[0]; sh[1] = P[1]; sh[2] = P[2]; sh[3] = P[3];
      break;
    case SIDE_XN:
      sh[0] = P[4]; sh[1] = P[5]; sh[2] = P[6]; sh[3] = P[7];
      break;
    case SIDE_YP:
      sh[0] = P[4]; sh[1] = P[1]; sh[2] = P[0]; sh[3] = P[5];
      break;
    case SIDE_YN:
      sh[0] = P[6]; sh[1] = P[3]; sh[2] = P[2]; sh[3] = P[7];
      break;
    case SIDE_ZP:
      sh[0] = P[1]; sh[1] = P[4]; sh[2] = P[7]; sh[3] = P[2];
      break;
    case SIDE_ZN:
      sh[0] = P[5]; sh[1] = P[0]; sh[2] = P[3]; sh[3] = P[6];
  }

  size_t i = side * digits_per_side;
  for(u_int v = 0; v < vertices_per_side; ++v)
  {
    data[i + X] = sh[v].x + Origin.x;
    data[i + Y] = sh[v].y + Origin.y;
    data[i + Z] = sh[v].z + Origin.z;
    i += digits_per_vertex;
  }
}

///
/// \brief voxel::init_arrays
///
void voxel::init_data(void)
{
  color IniColor {1.f, 1.f, 1.f, 1.f};

  for (u_int side = 0; side < SIDES_COUNT; ++side)
  {
    vbo_addr[side]  = -1;
    visible[side] = true;
    tex_id[side]  = {7, 5};

    side_position_set(side);
    side_color_set(side, IniColor);
    side_normals_set(side);
    side_texture_set(side);
  }
}


///
/// \brief voxel::side_fill_data
/// \param s
/// \details Заполнение массива стороны данными. Если сторона
/// скрытая, то данные не записываются и возвращается false
///
bool voxel::side_fill_data(u_char side, GLfloat* buff)
{
  if(!visible[side]) return false;
  GLfloat* src = &data[side * digits_per_side];
  memcpy(buff, src, bytes_per_side);
  return true;
}


///
/// \brief voxel::offset_write
/// \param side_id
/// \param n
/// \details Запись адреса размещения данных в VBO стороны вокселя
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

} //tr
