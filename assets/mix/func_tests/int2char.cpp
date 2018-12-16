/* Кодирование целого через char */
#include <iostream>

int main(int, char**)
{
  int b = 1222555;
  int r, g;
  r  = b % 255;
  b /= 255;
  g  = b % 255;
  b /= 255;

  std::cout << "r + g * 255 + b * 255 * 255 = "
            << (r + g * 255 + b * 255 * 255) << "\n";

  return 0;
}
