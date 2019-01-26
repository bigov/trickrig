#include "box.hpp"

namespace tr
{

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
  for (size_t i = 0; i < SIDES_COUNT; ++i) {
    Sides[i].splice = splice_calc(i);
  }
}


///
/// \brief box::splice_calc
/// \param i
/// \return
///
v_fl box::splice_calc(size_t idx)
{
  v_fl res {};
  side S = Sides[idx]; // текущая сторона

  v_uch id = { // Индексы вершин, используемых для расчета сплайса стороны
    S.indexes[0], S.indexes[1],
    S.indexes[2], S.indexes[4],
  };

  switch (idx) {
    case SIDE_XP:
      for(u_char i = 0; i < 4; ++i)
      {
        if(vertex[id[i]][0] != 1.0f) return v_fl {}; // если вершина не на границе - сплайс пустой
        res[i*2]   = vertex[id[i]][1];  // y
        res[i*2+1] = vertex[id[i]][2];  // z
      }
      break;
    case SIDE_XN:
      std::swap(id[0], id[1]);
      std::swap(id[2], id[3]);
      for(u_char i = 0; i < 4; ++i)
      {
        if(vertex[id[i]][0] !=-1.0f) return v_fl {}; // если вершина не на границе - сплайс пустой
        res[i*2]   = vertex[id[i]][1];  // y
        res[i*2+1] = vertex[id[i]][2];  // z
      }
      break;
    case SIDE_YP:
      for(u_char i = 0; i < 4; ++i)
      {
        if(vertex[id[i]][1] != 1.0f) return v_fl {}; // если вершина не на границе - сплайс пустой
        res[i*2]   = vertex[id[i]][0];  // x
        res[i*2+1] = vertex[id[i]][2];  // z
      }
      break;
    case SIDE_YN:
      std::swap(id[0], id[1]);
      std::swap(id[2], id[3]);
      for(u_char i = 0; i < 4; ++i)
      {
        if(vertex[id[i]][1] !=-1.0f) return v_fl {}; // если вершина не на границе - сплайс пустой
        res[i*2]   = vertex[id[i]][0];  // x
        res[i*2+1] = vertex[id[i]][2];  // z
      }
      break;
    case SIDE_ZP:
      for(u_char i = 0; i < 4; ++i)
      {
        if(vertex[id[i]][2] != 1.0f) return v_fl {}; // если вершина не на границе - сплайс пустой
        res[i*2]   = vertex[id[i]][0];  // x
        res[i*2+1] = vertex[id[i]][1];  // y
      }
      break;
    case SIDE_ZN:
      std::swap(id[0], id[1]);
      std::swap(id[2], id[3]);
      for(u_char i = 0; i < 4; ++i)
      {
        if(vertex[id[i]][2] !=-1.0f) return v_fl {}; // если вершина не на границе - сплайс пустой
        res[i*2]   = vertex[id[i]][0];  // x
        res[i*2+1] = vertex[id[i]][1];  // y
      }
      break;
    default:
      break;
  }
  return res;
}

}
