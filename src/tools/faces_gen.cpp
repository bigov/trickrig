#include <iostream>
#include "vox.hpp"

using namespace tr;


int main(int, char**)
{
  std::ofstream data_file("data.txt");

  int base_len = 256;
  i3d P {0, 0, 0};
  vox V {P, base_len};

  size_t offset = 0;
  for(int face = 0; face < SIDES_COUNT; face++) // стороны
  {
    for(int vert = 0; vert < 4; vert++)
    {
      for(size_t d = 0; d < digits_per_vertex; d++)
      {
        data_file << V.data[ offset++ ] << ", ";
      }
      data_file << std::endl;
    }
    data_file << std::endl;
  }
  return EXIT_SUCCESS;
}
