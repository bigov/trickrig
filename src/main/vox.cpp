#include "vox.hpp"

namespace tr
{


face_t face_gen(const i3d& Point, int side_len, uchar face)
{
  vox V {Point, side_len, face};
  return V.Face;
}


///
/// \brief vox::vox
/// \param Point
/// \param side_len
/// \param face
///
vox::vox(const i3d& Point, int side_len, uchar face): Origin(Point), side_len(side_len)
{
  /*
  switch (face) {
    case SIDE_XP:
      tex_id[face]  = {1, 7};
      break;
    case SIDE_XN:
      tex_id[face]  = {1, 6};
      break;
    case SIDE_YP:
      tex_id[face]  = {1, 5};
      break;
    case SIDE_YN:
      tex_id[face]  = {1, 4};
      break;
    case SIDE_ZP:
      tex_id[face]  = {1, 3};
      break;
    case SIDE_ZN:
      tex_id[face]  = {1, 2};
      break;
  }
  */
  tex_id[face]  = {0, 0};  // Назначение (u,v) координат текстуры из текстурной карты
  position_set(face);
  color_set(DefaultColor);
  normals_set(face);
  texture_set(face);
  Face[0] = face;
  memcpy(Face.data() + 1, data, bytes_per_face);
}


///
/// \brief voxel::side_position_set
/// \param side
///
void vox::position_set(uchar face)
{
  int l = - side_len;
  int O = 0;

  /**     (7)--------(0)
       Yp /|         /|
        |/ |        / |
       (4)--------(3) | Zp
        |  |       |  |/
        | (6)------|-(1)
        | /        | /
        |/         |/
   Xn--(5)--------(2)--Xp
        |         /
       Yn       Zn

       X, Y, Z        вершины
    0: 0, 0, 0    Xp: 0,1,2,3
    1: 0,-1, 0    Xn: 4,5,6,7
    2: 0,-1,-1    Yp: 0,3,4,7
    3: 0, 0,-1    Yn: 1,6,5,2
    4:-1, 0,-1    Zp: 0,7,6,1
    5:-1,-1,-1    Zn: 2,5,4,3
    6:-1,-1, 0
    7:-1, 0, 0                       */

  // В начале координат находится вершина 0
  i3d P[8] = {{ O, O, O }, { O, l, O }, { O, l, l }, { O, O, l },
              { l, O, l }, { l, l, l }, { l, l, O }, { l, O, O }};

  i3d sh[4]; // смещения 4-х образующих вершин выбранной стороны вокселя
  switch (face) {
    case SIDE_XP:
      sh[0] = P[0]; sh[1] = P[1]; sh[2] = P[2]; sh[3] = P[3];
      break;
    case SIDE_XN:
      sh[0] = P[4]; sh[1] = P[5]; sh[2] = P[6]; sh[3] = P[7];
      break;
    case SIDE_YP:
      sh[0] = P[0]; sh[1] = P[3]; sh[2] = P[4]; sh[3] = P[7];
      break;
    case SIDE_YN:
      sh[0] = P[1]; sh[1] = P[6]; sh[2] = P[5]; sh[3] = P[2];
      break;
    case SIDE_ZP:
      sh[0] = P[0]; sh[1] = P[7]; sh[2] = P[6]; sh[3] = P[1];
      break;
    case SIDE_ZN:
      sh[0] = P[2]; sh[1] = P[5]; sh[2] = P[4]; sh[3] = P[3];
  }

  size_t i = 0;
  for(uint v = 0; v < vertices_per_face; ++v)
  {
    data[i + X] = static_cast<float>(sh[v].x + Origin.x);
    data[i + Y] = static_cast<float>(sh[v].y + Origin.y);
    data[i + Z] = static_cast<float>(sh[v].z + Origin.z);
    i += digits_per_vertex;
  }
}


///
/// \brief voxel::side_color_set
/// \param side
/// \param C
///
void vox::color_set(color C)
{
  size_t i = 0;
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
void vox::texture_set(uchar face)
{
  uch2 texture = tex_id[face];
  size_t i =  0;

  switch (face) {
    case SIDE_XP:
    case SIDE_XN:
    case SIDE_YP:
      data[i + U] = u_sz * texture.u + u_sz;
      data[i + V] = v_sz * texture.v;
      i += digits_per_vertex;
      data[i + U] = u_sz * texture.u + u_sz;
      data[i + V] = v_sz * texture.v + v_sz;
      i += digits_per_vertex;
      data[i + U] = u_sz * texture.u;
      data[i + V] = v_sz * texture.v + v_sz;
      i += digits_per_vertex;
      data[i + U] = u_sz * texture.u;
      data[i + V] = v_sz * texture.v;
      break;
    case SIDE_YN:
    case SIDE_ZN:
      data[i + U] = u_sz * texture.u + u_sz;
      data[i + V] = v_sz * texture.v + v_sz;
      i += digits_per_vertex;
      data[i + U] = u_sz * texture.u;
      data[i + V] = v_sz * texture.v + v_sz;
      i += digits_per_vertex;
      data[i + U] = u_sz * texture.u;
      data[i + V] = v_sz * texture.v;
      i += digits_per_vertex;
      data[i + U] = u_sz * texture.u + u_sz;
      data[i + V] = v_sz * texture.v;
      break;
    case SIDE_ZP:
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
      break;
  }

}


///
/// \brief voxel::side_normals_set
/// \param side
///
void vox::normals_set(uchar face)
{
  GLfloat nx = 0.f, ny = 0.f, nz = 0.f;

  switch (face) {
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

  size_t i =  0;
  for(uint v = 0; v < vertices_per_face; ++v)
  {
    data[i + NX] = nx;
    data[i + NY] = ny;
    data[i + NZ] = nz;
    i += digits_per_vertex;
  }
}

} //tr
