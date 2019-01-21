/*
 *
 *
 *
 */

#include "rig.hpp"

namespace tr
{


///
/// \brief rig::rig
/// \param Other
///
rig::rig(const tr::rig & Other)
{
  copy_data(Other);
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
    copy_data(Other);
  }
  return *this;
}


///
/// \brief rig::copy_sides
/// \param Other
///
void rig::copy_data(const rig &Other)
{
  born = Other.born;
  Origin = Other.Origin;
  for(int i = 0; i < SHIFT_DIGITS; i++) shift[i] = Other.shift[i];

  SideXp.clear();
  SideXn.clear();
  SideYp.clear();
  SideYn.clear();
  SideZp.clear();
  SideZn.clear();

  SideXp = Other.SideXp;
  SideXn = Other.SideXn;
  SideYp = Other.SideYp;
  SideYn = Other.SideYn;
  SideZp = Other.SideZp;
  SideZn = Other.SideZn;
}

} //tr
