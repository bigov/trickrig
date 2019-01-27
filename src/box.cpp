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
box::box(f3d& V, float l)
{
  lod = l;
  Vertex.push_back(f3d{ V.x    , V.y + l, V.z + l });
  Vertex.push_back(f3d{ V.x + l, V.y + l, V.z + l });
  Vertex.push_back(f3d{ V.x + l, V.y + l, V.z     });
  Vertex.push_back(f3d{ V.x    , V.y + l, V.z     });
  Vertex.push_back(f3d{ V.x    , V.y    , V.z + l });
  Vertex.push_back(f3d{ V.x + l, V.y    , V.z + l });
  Vertex.push_back(f3d{ V.x + l, V.y    , V.z     });
  Vertex.push_back(f3d{ V.x    , V.y    , V.z     });
  init_arrays();
}


///
/// \brief box::box
/// \param V
///
box::box(std::vector<f3d>& V)
{
  size_t n = V.size();
  if( n != 8 )
  {
    std::cout << "ERR: Box require 8 verices\n";
    return;
  }
  Vertex = V;
  init_arrays();
}


///
/// \brief box::init_arrays
///
void box::init_arrays(void)
{
  Indexes = { // индексы вершин, образующих сторону
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

  Color = {{1.0f, 1.0f, 1.0f, 1.0f}}; //Если Color.size() > 1, то цвета вершин различаются
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
/// \brief box::visible
/// \param s
/// \param V1
/// \return Отображать сторону или нет
///
/// \details сторона отображается если сплайсы не совпадют
///
bool box::visible(SIDES s, splice& V1)
{
  return (Splice[s] != V1);
}


///
/// \brief box::splice_side_xp
///
void box::splice_side_xp(SIDES s)
{
  a_uch4 id = Indexes[s]; // Индексы вершин для расчета сплайса

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
  a_uch4 id = { Indexes[s][1], Indexes[s][0], Indexes[s][3], Indexes[s][2] };

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
  a_uch4 id = Indexes[s];

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
  a_uch4 id = { Indexes[s][1], Indexes[s][0], Indexes[s][3], Indexes[s][2] };

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
  a_uch4 id = Indexes[s];

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
  a_uch4 id = { Indexes[s][1], Indexes[s][0], Indexes[s][3], Indexes[s][2] };

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
