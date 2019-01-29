#include <iostream>
#include <array>
#include <vector>

struct v_color {float r=1.0f, g=1.1f, b=1.2f, a=0.f;};  // цвет вершины - 4 числа
using s_color = std::array <v_color, 4>;                // сторона - 4 вершины по 4 числа

int main(int, char**)
{
  s_color Side0 {v_color{}, {}, {}, {}}; // цвет стороны - цвета 4-х вершин
  s_color s[6] = {Side0};                // шесть сторон
  std::array <s_color*, 6> aSides  { &s[0], &s[0], &s[0], &s[0] };

  int side_id = 2;
  int vertex_id = 3;

  s_color* mySide = aSides[side_id]; // цвета стороны
  s_color Vertex_array = mySide[3];
  float c = Vertex_array[vertex_id].r;

  std::cout
     //<< aSides[1][Side][Vert].b
     << "\n ok \n";

  return EXIT_SUCCESS;
}
