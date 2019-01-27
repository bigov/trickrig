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
///
box::box(std::vector<ar_f3>& V)
{
  size_t n = V.size();
  if( n != 8 )
  {
    std::cout << "ERR: Box require 8 verices\n";
    return;
  }
  vertex = V;

  splice_side_xp();
  splice_side_xn();
  splice_side_yp();
  splice_side_yn();
  splice_side_zp();
  splice_side_zn();
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
  return (Sides[s].Splice != V1);
}


///
/// \brief box::splice_side_xp
///
void box::splice_side_xp(void)
{
  side S = Sides[SIDE_XP];

  // Индексы вершин, используемых для расчета сплайса
  v_uch id = { S.Indexes[0], S.Indexes[1], S.Indexes[2], S.Indexes[4] };

  size_t n = id.size();
  S.Splice.clear();
  S.Splice.resize(n);

  bool all_top = true;
  for(u_char i = 0; i < n; ++i)
  {
    if(vertex[id[i]][X] != 1.0f)  all_top = false;
    S.Splice[i*2]   = &vertex[id[i]][Y];
    S.Splice[i*2+1] = &vertex[id[i]][Z];
  }
  if(!all_top) S.Splice.clear();
}


///
/// \brief box::splice_side_xn
///
void box::splice_side_xn(void)
{
  side S = Sides[SIDE_XN];

  // Индексы вершин, используемых для расчета сплайса обратной стороны
  // выбираются в обратном направлении
  v_uch id = { S.Indexes[1], S.Indexes[0], S.Indexes[4], S.Indexes[2] };

  size_t n = id.size();
  S.Splice.clear();
  S.Splice.resize(n);

  bool all_top = true;
  for(u_char i = 0; i < n; ++i)
  {
    if(vertex[id[i]][X] !=-1.0f)  all_top = false;
    S.Splice[i*2]   = &vertex[id[i]][Y];
    S.Splice[i*2+1] = &vertex[id[i]][Z];
  }
  if(!all_top) S.Splice.clear();
}


///
/// \brief box::splice_side_yp
///
void box::splice_side_yp(void)
{
  side S = Sides[SIDE_YP];

  // Индексы вершин, используемых для расчета сплайса
  v_uch id = { S.Indexes[0], S.Indexes[1], S.Indexes[2], S.Indexes[4] };

  size_t n = id.size();
  S.Splice.clear();
  S.Splice.resize(n);

  bool all_top = true;
  for(u_char i = 0; i < n; ++i)
  {
    if(vertex[id[i]][Y] != 1.0f)  all_top = false;
    S.Splice[i*2]   = &vertex[id[i]][X];
    S.Splice[i*2+1] = &vertex[id[i]][Z];
  }
  if(!all_top) S.Splice.clear();
}


///
/// \brief box::splice_side_yn
///
void box::splice_side_yn(void)
{
  side S = Sides[SIDE_YN];

  // Индексы вершин, используемых для расчета сплайса обратной стороны
  // выбираются в обратном направлении
  v_uch id = { S.Indexes[1], S.Indexes[0], S.Indexes[4], S.Indexes[2] };

  size_t n = id.size();
  S.Splice.clear();
  S.Splice.resize(n);

  bool all_top = true;
  for(u_char i = 0; i < n; ++i)
  {
    if(vertex[id[i]][Y] !=-1.0f)  all_top = false;
    S.Splice[i*2]   = &vertex[id[i]][X];
    S.Splice[i*2+1] = &vertex[id[i]][Z];
  }
  if(!all_top) S.Splice.clear();
}


///
/// \brief box::splice_side_zp
///
void box::splice_side_zp(void)
{
  side S = Sides[SIDE_ZP];

  // Индексы вершин, используемых для расчета сплайса
  v_uch id = { S.Indexes[0], S.Indexes[1], S.Indexes[2], S.Indexes[4] };

  size_t n = id.size();
  S.Splice.clear();
  S.Splice.resize(n);

  bool all_top = true;
  for(u_char i = 0; i < n; ++i)
  {
    if(vertex[id[i]][Z] != 1.0f) all_top = false;
    S.Splice[i*2]   = &vertex[id[i]][X];
    S.Splice[i*2+1] = &vertex[id[i]][Y];
  }
  if(!all_top) S.Splice.clear();
}


///
/// \brief box::splice_side_zn
///
void box::splice_side_zn(void)
{
  side S = Sides[SIDE_ZN];

  // Индексы вершин, используемых для расчета сплайса обратной стороны
  // выбираются в обратном направлении
  v_uch id = { S.Indexes[1], S.Indexes[0], S.Indexes[4], S.Indexes[2] };

  size_t n = id.size();
  S.Splice.clear();
  S.Splice.resize(n);

  bool all_top = true;
  for(u_char i = 0; i < n; ++i)
  {
    if(vertex[id[i]][Z] !=-1.0f) all_top = false;
    S.Splice[i*2]   = &vertex[id[i]][X];
    S.Splice[i*2+1] = &vertex[id[i]][Y];
  }
  if(!all_top) S.Splice.clear();
}

} //tr
