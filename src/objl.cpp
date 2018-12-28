/**
  * file: loader_obj.cpp
  *
  * Класс загрузчик-парсер 3D файлов формата .obj
  *
  * Результат парсера размещается в массиве "vertices", который представляет
  * из себя список записей (std::vector) из пар массивов фиксированного размера.
  *
  * Первый элемент каждой пары - это три координаты вершины, второй - три
  * координаты нормали к этой вершине. Все числа - в формате float.
  */

#include "objl.hpp"

namespace tr {

//## конструктор класса построчно считывает файл
obj_load::obj_load(const std::string &FName)
{
  std::string LineBuffer {}; // используем в качестве динамического буфера
  std::ifstream FStream{ FName, std::ios::binary | std::ios::ate };

  if( FStream )
  {
    auto size = FStream.tellg(); size += 1;
    LineBuffer.resize(size, '\0'); // размер буфера в размер файла
    FStream.seekg(0); size -= 1;
    while (FStream.getline(&LineBuffer[0], size)) parsing_line(&LineBuffer[0]);
  } else { std::cout << "Can't open file"; }

  Places.clear();
  Normals.clear();
}

//## переводит строку вида "0/0/0" в числовой массив
void obj_load::decode_string(const std::string & f, int *a)
{
 /***
  * 1. Если индексы текстурных координат отсутуствуют, то используются
  * два значения с разделителем из двух слэшей, иначе блок данных
  * состоит из трех целых, разделеных одним слэшем.
  *
  * 2. В стандарте .obj все индексы начинаются с единицы, а не с нуля,
  * как в C++ массивах. Поэтому при формировании пары, чтобы
  * получить целевой индекс, уменьшаем полученное число на 1.
  */

  std::string::size_type sz = 0, cur = 0;

  if (f.find("//") != std::string::npos)
  {
    a[0] = std::stoi(f, &sz) - 1;
    a[1] = 0; //auto idx1 = ... индексы текстурной карты отсутствуют
    a[2] = std::stoi(f.substr(sz + 2)) - 1;
  } else
  {
    textured = true;
    a[0] = std::stoi(f.substr(cur), &sz) - 1; cur += sz + 1;
    a[1] = std::stoi(f.substr(cur), &sz) - 1; cur += sz + 1;
    a[2] = std::stoi(f.substr(cur)     ) - 1;
  }
}

//## прием 4-х групп индексов, образующих прямоугольник
void obj_load::get_f(char *line)
{
  int i[4][3]; //4 вершины: индексы массивов координат, текстур и нормалей

  decode_string(std::string(std::strtok(line, " ")), i[0]);
  decode_string(std::string(std::strtok(NULL, " ")), i[1]);
  decode_string(std::string(std::strtok(NULL, " ")), i[2]);
  decode_string(std::string(std::strtok(NULL, " ")), i[3]);

  tr::snip Snip {};

  for(size_t n = 0; n < tr::vertices_per_snip; n++)
  {
    Snip.data[n*ROW_SIZE + X] = Places[i[n][0]][0];
    Snip.data[n*ROW_SIZE + Y] = Places[i[n][0]][1];
    Snip.data[n*ROW_SIZE + Z] = Places[i[n][0]][2];

    if(textured) {
      Snip.data[n*ROW_SIZE + U] = UVs[i[n][1]][0];
      Snip.data[n*ROW_SIZE + V] = UVs[i[n][1]][1];
    }

    Snip.data[n*ROW_SIZE + NX] = Normals[i[n][2]][0];
    Snip.data[n*ROW_SIZE + NY] = Normals[i[n][2]][1];
    Snip.data[n*ROW_SIZE + NZ] = Normals[i[n][2]][2];
  }

  Area.push_front(Snip);
}

//## прием трех чисел в формате float в список нормалей
void obj_load::get_vn(char *line)
{
  char *token = std::strtok(line, " ");
  std::array<float, 3> normal {};
  for(size_t i = 0; i < 3; i++)
  {
    normal[i] = std::stof(token);
    token = std::strtok(NULL, " ");
  }
  Normals.push_back(normal);
}

//## прием двух чисел в формате float в список UV текстуры
void obj_load::get_vt(char *line)
{
  std::array<float, 2> UV {};
  UV[0] = std::stof(std::strtok(line, " "));
  UV[1] = std::stof(std::strtok(NULL, " "));
  UVs.push_back(UV);
}

//## прием трех чисел в формате float в список вершин
void obj_load::get_v(char *line)
{
  char *token = std::strtok(line, " ");
  std::array<float, 3> Place;
  for(size_t i = 0; i < 3; i++)
  {
    Place[i] = std::stof(token);
    token = std::strtok(NULL, " ");
  }
  Places.push_back(Place);
}

//## Разбор типа переданой строки символов по ключу
void obj_load::parsing_line(char *line)
{
  std::string LineId = std::strtok(line, " ");
  if(LineId == "v") get_v(&line[2]);
  else if(LineId == "vn") get_vn(&line[3]);
  else if(LineId == "vt") get_vt(&line[3]);
  else if(LineId == "f") get_f(&line[2]);
}

} //namespace tr
