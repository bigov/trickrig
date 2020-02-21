#include <iostream>
#include <vector>

using uchar = unsigned char;
using uint  = unsigned int;
using ulong = unsigned long;

const int bytes_per_side = 24;

struct side_data
{
  uchar id = 6;                   // Определение стороны (Xp|Xn|Yp|Yn|Zp|Zn)
  uchar vbo_data[bytes_per_side] = {'\0'}; // Данные вершин, записываемые в VBO (uchar d[])
};

// Структура для работы с форматом данных воксов
struct vox_data
{
  int y = 0;                       // Y-координата вокса
  std::vector<side_data> Sides {}; // Массив сторон
};

struct data_pack
{
  int x = 0;
  int z = 0;                      // координаты ячейки
  std::vector<vox_data> Voxes {}; // Массив воксов, хранящихся в БД
};

int main(int, char**)
{
  std::cout << "---\n" << std::endl;

  side_data Side {};
  vox_data Vox {};
  Side.id = 1;
  Vox.Sides.emplace_back(Side);
  Side.id = 3;
  Vox.Sides.emplace_back(Side);

  for(auto& S: Vox.Sides)
  {
    int id = Side.id;
    std::cout << "Side.id = "<< id << std::endl;
  }

  std::cout << "\nCompleted" << std::endl;
  return EXIT_SUCCESS;
}
