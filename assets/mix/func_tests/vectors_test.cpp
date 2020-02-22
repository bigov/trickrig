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
  for(int i = 1; i < 4; ++i)
  {
    Side.id = i;
    Vox.Sides.push_back(Side);
  }

  for(auto& S: Vox.Sides)
  {
    int id = S.id;
    std::cout << "Side.id = "<< id << std::endl;
  }
  data_pack DataPack {};
  DataPack.Voxes.push_back(Vox);

  std::cout << DataPack.Voxes.size();

  std::cout << "\nГотово!" << std::endl;
  return EXIT_SUCCESS;
}
