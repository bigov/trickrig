#include <iostream>
#include <array>
#include <vector>

#define S_XP 0
#define S_XN 1
#define S_YP 2
#define S_YN 3
#define S_ZP 4
#define S_ZN 5


struct color {float r=1.0f, g=1.1f, b=1.2f, a=0.f;};  // цвет вершины - 4 числа
using side = std::array <color, 4>;            // сторона - 4 вершины по 4 числа
using u_char = unsigned char;

int main(int, char**)
{
  std::vector<side> Box {};
  Box.push_back({color{}, {}, {}, {}});            // одна сторона - 4 вершины
  std::array<u_char, 6> side_cursor {0,0,0,0,0,0}; // шесть сторон одного цвета

  int vertex_id = 3;

  float c = Box[side_cursor[S_XN]][vertex_id].g ; // сторона

  std::cout << c << "\n ok \n";

  return EXIT_SUCCESS;
}
