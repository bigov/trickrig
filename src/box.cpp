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
  Vertex.push_back(f3d{ B.x     , B.y + ly, B.z + lz });
  Vertex.push_back(f3d{ B.x + lx, B.y + ly, B.z + lz });
  Vertex.push_back(f3d{ B.x + lx, B.y + ly, B.z      });
  Vertex.push_back(f3d{ B.x     , B.y + ly, B.z      });
  Vertex.push_back(f3d{ B.x     , B.y     , B.z + lz });
  Vertex.push_back(f3d{ B.x + lx, B.y     , B.z + lz });
  Vertex.push_back(f3d{ B.x + lx, B.y     , B.z      });
  Vertex.push_back(f3d{ B.x     , B.y     , B.z      });
  init_arrays();
}


///
/// \brief box::box
/// \param V
/// \details Конструктор бокса по готовому набору из 8 вершин
///
box::box(const std::array<f3d, 8>& Arr)
{
  Vertex.clear();
  for(f3d Coord: Arr) Vertex.push_back(Coord);
  init_arrays();
}


///
/// \brief box::init_arrays
///
void box::init_arrays(void)
{
  SideIdx = { // индексы вершин, образующих сторону
    a_uch4{ 2, 1, 5, 6 },
    a_uch4{ 0, 3, 7, 4 },
    a_uch4{ 0, 1, 2, 3 },
    a_uch4{ 7, 6, 5, 4 },
    a_uch4{ 1, 0, 4, 5 },
    a_uch4{ 3, 2, 6, 7 }
  };

  Normals = { // Направление нормалей по сторонам
    a_f3{ 1.0f, 0.0f, 0.0f },
    a_f3{-1.0f, 0.0f, 0.0f },
    a_f3{ 0.0f, 1.0f, 0.0f },
    a_f3{ 0.0f,-1.0f, 0.0f },
    a_f3{ 0.0f, 0.0f, 1.0f },
    a_f3{ 0.0f, 0.0f,-1.0f }
  };

  //Color = {{1.0f, 1.0f, 1.0f, 1.0f}}; //Если Color.size() > 1, то цвета вершин различаются
  // Если Texture.size() = 1, то у всех сторон одинаковая текстура
  Texture = { a_f2{0.0f, 0.0f}, a_f2{u_sz, 0.0f}, a_f2{u_sz, u_sz}, a_f2{0.0f, u_sz} };

  splice_side_xp(SIDE_XP);
  splice_side_xn(SIDE_XN);
  splice_side_yp(SIDE_YP);
  splice_side_yn(SIDE_YN);
  splice_side_zp(SIDE_ZP);
  splice_side_zn(SIDE_ZN);
}


///
/// \brief box::side_data
/// \param s
/// \details Заполнение массива данными
///
void box::side_data(SIDES s, std::array<GLfloat, digits_per_snip>& data)
{
  for(size_t n = 0; n < vertices_per_snip; n++)
  {
    f3d V = Vertex[SideIdx[s][n]];
    auto N = Normals[s][n];
    auto C = Color[s][n];

    data[ROW_SIZE * n + X] = V.x;
    data[ROW_SIZE * n + Y] = V.y;
    data[ROW_SIZE * n + Z] = V.x;

    data[ROW_SIZE * n + R] = C[0];
    data[ROW_SIZE * n + G] = C[1];
    data[ROW_SIZE * n + B] = C[2];
    data[ROW_SIZE * n + A] = C[3];

  }


}


///
/// \brief box::visible
/// \param s
/// \param V1
/// \return Отображать сторону или нет
///
/// \details сторона отображается если сплайсы не совпадют
///
bool box::is_visible(SIDES s, splice& V1)
{
  return (Splice[s] != V1);
}


///
/// \brief box::splice_side_xp
///
void box::splice_side_xp(SIDES s)
{
  a_uch4 id = SideIdx[s]; // Индексы вершин для расчета сплайса

  size_t n = id.size();
  Splice[s].clear();
  Splice[s].resize(n);

  bool all_top = true;
  for(u_char i = 0; i < n; ++i)
  {
    if(Vertex[id[i]].x != 1.0f)  all_top = false;
    Splice[s][i*2]   = &Vertex[id[i]].y;
    Splice[s][i*2+1] = &Vertex[id[i]].z;
  }
  if(!all_top) Splice[s].clear();
}


///
/// \brief box::splice_side_xn
///
void box::splice_side_xn(SIDES s)
{
  // Индексы вершин, используемых для расчета сплайса
  // обратной стороны выбираются в обратном направлении
  a_uch4 id = { SideIdx[s][1], SideIdx[s][0], SideIdx[s][3], SideIdx[s][2] };

  size_t n = id.size();
  Splice[s].clear();
  Splice[s].resize(n);

  bool all_top = true;
  for(u_char i = 0; i < n; ++i)
  {
    if(Vertex[id[i]].x !=-1.0f)  all_top = false;
    Splice[s][i*2]   = &Vertex[id[i]].y;
    Splice[s][i*2+1] = &Vertex[id[i]].z;
  }
  if(!all_top) Splice[s].clear();
}


///
/// \brief box::splice_side_yp
///
void box::splice_side_yp(SIDES s)
{
  // Индексы вершин, используемых для расчета сплайса
  a_uch4 id = SideIdx[s];

  size_t n = id.size();
  Splice[s].clear();
  Splice[s].resize(n);

  bool all_top = true;
  for(u_char i = 0; i < n; ++i)
  {
    if(Vertex[id[i]].y != 1.0f)  all_top = false;
    Splice[s][i*2]   = &Vertex[id[i]].x;
    Splice[s][i*2+1] = &Vertex[id[i]].z;
  }
  if(!all_top) Splice[s].clear();
}


///
/// \brief box::splice_side_yn
///
void box::splice_side_yn(SIDES s)
{
  // Индексы вершин, используемых для расчета сплайса обратной стороны
  // выбираются в обратном направлении
  a_uch4 id = { SideIdx[s][1], SideIdx[s][0], SideIdx[s][3], SideIdx[s][2] };

  size_t n = id.size();
  Splice[s].clear();
  Splice[s].resize(n);

  bool all_top = true;
  for(u_char i = 0; i < n; ++i)
  {
    if(Vertex[id[i]].y !=-1.0f)  all_top = false;
    Splice[s][i*2]   = &Vertex[id[i]].x;
    Splice[s][i*2+1] = &Vertex[id[i]].z;
  }
  if(!all_top) Splice[s].clear();
}


///
/// \brief box::splice_side_zp
///
void box::splice_side_zp(SIDES s)
{
  // Индексы вершин, используемых для расчета сплайса
  a_uch4 id = SideIdx[s];

  size_t n = id.size();
  Splice[s].clear();
  Splice[s].resize(n);

  bool all_top = true;
  for(u_char i = 0; i < n; ++i)
  {
    if(Vertex[id[i]].z != 1.0f) all_top = false;
    Splice[s][i*2]   = &Vertex[id[i]].x;
    Splice[s][i*2+1] = &Vertex[id[i]].y;
  }
  if(!all_top) Splice[s].clear();
}


///
/// \brief box::splice_side_zn
///
void box::splice_side_zn(SIDES s)
{
  // Индексы вершин, используемых для расчета сплайса обратной стороны
  // выбираются в обратном направлении
  a_uch4 id = { SideIdx[s][1], SideIdx[s][0], SideIdx[s][3], SideIdx[s][2] };

  size_t n = id.size();
  Splice[s].clear();
  Splice[s].resize(n);

  bool all_top = true;
  for(u_char i = 0; i < n; ++i)
  {
    if(Vertex[id[i]].z !=-1.0f) all_top = false;
    Splice[s][i*2]   = &Vertex[id[i]].x;
    Splice[s][i*2+1] = &Vertex[id[i]].y;
  }
  if(!all_top) Splice[s].clear();
}

} //tr
