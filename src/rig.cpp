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

  Boxes.clear();
  Boxes = Other.Boxes;
}


} //tr
