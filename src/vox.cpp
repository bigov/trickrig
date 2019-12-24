/**
 *
 *
 *
 */

#include "vox.hpp"

namespace tr
{
///
/// \brief voxel::box
/// \param Origin point
/// \param Side length
///
vox::vox(const i3d& Or, int sz)
: Origin(Or), side_len(sz), born(tr::get_msec())
{
  init_data();
}


///
/// \brief voxel::side_color_set
/// \param side
/// \param C
///
void vox::side_color_set(uint8_t side, color C)
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
void vox::side_texture_set(uint8_t side)
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
void vox::side_normals_set(uint8_t side)
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
void vox::side_position_set(uint8_t side)
{
  // относительные координаты всех вершин вокселя
  i3d P[8] = {{side_len, side_len, 0}, {side_len, side_len, side_len}, {side_len, 0, side_len},
              {side_len, 0, 0}, {0, side_len, side_len}, {0, side_len, 0}, {0, 0, 0}, {0, 0, side_len}};

  i3d sh[4]; // смещения 4-х образующих вершин выбранной стороны вокселя
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
void vox::init_data(void)
{
  color IniColor {1.f, 1.f, 1.f, 1.f};

  for (u_int side = 0; side < SIDES_COUNT; ++side)
  {
    vbo_addr[side]  = -1;
    visibility.set(side);
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
bool vox::side_fill_data(uint8_t side, GLfloat* buff)
{
  if(!is_visible(side)) return false;
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
void vox::offset_write(uint8_t side_id, GLsizeiptr n)
{
#ifndef NDEBUG
  if(side_id >= SIDES_COUNT) ERR ("voxel::offset_write: side_id >= SIDES_COUNT");
  if(!is_visible(side_id)) ERR("voxel::offset_write: unvisible side");
#endif

  vbo_addr[side_id] = n;
}


///
/// \brief voxel::side_id_by_offset
/// \return
/// \details По указанному смещению определяет какая сторона там находится
///
uint8_t vox::side_id_by_offset(GLsizeiptr dst)
{
  if(dst < 0) ERR("side_id_by_offset: undefined address");

  for (uint8_t side_id = 0; side_id < SIDES_COUNT; ++side_id) {
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
GLsizeiptr vox::offset_read(uint8_t side_id)
{
  if(!is_visible(side_id)) return -1;
  return vbo_addr[side_id];
}


///
/// \brief voxel::offset_replace
/// \param old_n
/// \param new_n
///
void vox::offset_replace(GLsizeiptr old_n, GLsizeiptr new_n)
{
  uint8_t side_id;
  for (side_id = 0; side_id < SIDES_COUNT; ++side_id)
  {
    if(vbo_addr[side_id] == old_n)
    {
      #ifndef NDEBUG
        if(!visibility.test(side_id)) info("voxel::offset_replace for unvisible side.");
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
/// \brief vox::visible_on
/// \param side_id
///
void vox::visible_on(uint8_t side_id)
{
#ifndef NDEBUG
  assert(side_id < SIDES_COUNT);
#endif

  visibility.set(side_id);
}


///
/// \brief vox::visible_off
/// \param side_id
///
void vox::visible_off(uint8_t side_id)
{
#ifndef NDEBUG
  assert(side_id < SIDES_COUNT);
#endif

  visibility.reset(side_id);
}


///
/// \brief vox::is_visible
/// \param side_id
/// \return
///
bool vox::is_visible(uint8_t side_id)
{
#ifndef NDEBUG
  assert(side_id < SIDES_COUNT);
#endif

  return visibility.test(side_id);
}


} //tr
