#include <iostream>

int main(int, char**)
{
  bool t[3] = {true};
  bool f[3] = {false};

  std::cout << t[0] << ", " << t[1] << ", " << t[2] << "\n";

  return EXIT_SUCCESS;
}
