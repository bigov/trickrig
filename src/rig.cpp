/*
 *
 *
 *
 */

#include "rig.hpp"

namespace tr
{

///
/// Дублирующий конструктор
///
rig::rig(const tr::rig & Other)
{
  born = Other.born;
  for(int i = 0; i < SHIFT_DIGITS; i++) shift[i] = Other.shift[i];
  Trick.clear();
  for(snip Snip: Other.Trick) Trick.push_front(Snip);
}


///
/// \brief rig::operator =
/// \param Other
/// \return
///
rig& rig::operator= (const rig &Other)
{
  if(this != &Other)
  {
    born = get_msec();
    for(int i = 0; i < SHIFT_DIGITS; i++) shift[i] = Other.shift[i];
    Trick.clear();
    for(snip Snip: Other.Trick) Trick.push_front(Snip);
  }
  return *this;
}

} //tr
