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
void vox::face_color_set(uchar side, color C)
{
  size_t i = side * digits_per_face;
  for(uint v = 0; v < vertices_per_face; ++v)
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
void vox::face_texture_set(uchar side)
{
  uch2 texture = tex_id[side];

  size_t i =  side * digits_per_face;
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
void vox::face_normals_set(uchar side)
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

  size_t i =  side * digits_per_face;
  for(uint v = 0; v < vertices_per_face; ++v)
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
void vox::face_position_set(uchar side)
{
  // относительные координаты всех вершин вокселя
  //int l = side_len/2;
  //int m = -l;
  int l = side_len;
  int m = 0;
  i3d P[8] = {{ l, l, m }, { l, l, l }, { l, m, l }, { l, m, m },
              { m, l, l }, { m, l, m }, { m, m, m }, { m, m, l }};

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

  size_t i = side * digits_per_face;
  for(uint v = 0; v < vertices_per_face; ++v)
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

  for (uint side = 0; side < SIDES_COUNT; ++side)
  {
    visibility.set(side);
    tex_id[side]  = {7, 5};

    face_position_set(side);
    face_color_set(side, IniColor);
    face_normals_set(side);
    face_texture_set(side);
  }
}


///
/// \brief voxel::side_fill_data
/// \param s
/// \details Заполнение массива стороны данными. Если сторона
/// скрытая, то данные не записываются и возвращается false
///
bool vox::face_fill_data(uchar side, GLfloat* buff)
{
  if(!is_visible(side)) return false;
  GLfloat* src = &data[side * digits_per_face];
  memcpy(buff, src, bytes_per_face);
  return true;
}


///
/// \brief vox::visible_on
/// \param side_id
///
void vox::visible_on(uchar side_id)
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
void vox::visible_off(uchar side_id)
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
bool vox::is_visible(uchar side_id)
{
#ifndef NDEBUG
  assert(side_id < SIDES_COUNT);
#endif

  return visibility.test(side_id);
}


} //tr
