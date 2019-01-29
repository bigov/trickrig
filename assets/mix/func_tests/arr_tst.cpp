#include <iostream>
#include <array>
#include <vector>

struct v_color {float r=1.0f, g=1.1f, b=1.2f, a=0.f;};  // цвет вершины - 4 числа
using s_color = std::array <v_color, 4>;

int main(int, char**)
{
  s_color SideC {v_color{}, {}, {}, {}}; // цвет стороны - цвета 4-х вершин

  std::vector<s_color> SidesClrs {s_color{}};
  std::array <s_color*, 4> Clrs  { &(SidesClrs[0]), &(SidesClrs[0]) };

  int Side = 0;
  int Vert = 0;

  s_color Ts = SidesClrs[0]; // цвета стороны
  v_color Tv = Ts[0];
  float c = Tv.r;

  std::cout
     << Clrs[1][Side][Vert].b
     << "\nok\n";

  return EXIT_SUCCESS;
}
