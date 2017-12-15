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

#include "loader_obj.hpp"

namespace tr {

//## конструктор класса построчно считывает файл
loader_obj::loader_obj(const std::string &FName, const tr::f3d &P): Point(P)
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
  return;
}

//## переводит строку вида "0/0/0" в числовой массив
void loader_obj::decode_string(const std::string & f, int *a)
{
 /***
  * 1. Так как индексы текстурных координат отсутуствуют,
  * то используются два значения с разделителем из двух слэшей.
  * Если нужна поддержка текстур, то блок данных будет состоять
  * из трех целых, разделеных одним слэшем.
  *
  * 2. В стандарте .obj индексы вершин фeйсов (f...) начинаются с единицы,
  * а не с нуля как в C++ массивах. Поэтому при формировании пары, чтобы
  * получить целевой индекс, уменьшаем полученное число на 1.
  */

  std::string::size_type sz;
  a[0] = std::stoi(f, &sz) - 1;
  a[1] = 0; //auto idx1 = ... индекс текстурной карты - не используется
  a[2] = std::stoi(f.substr(sz + 2)) - 1;

  return;
}

//## прием 4-х групп индексов, образующих прямоугольник
void loader_obj::get_f(char *line)
{
  int iA[3], //вершина 1: индексы массива координат, (текстур) и нормалей
      iB[3], //вершина 2: индексы массива координат, (текстур) и нормалей
      iC[3], //вершина 3: индексы массива координат, (текстур) и нормалей
      iD[3]; //вершина 4: индексы массива координат, (текстур) и нормалей

  decode_string(std::string(std::strtok(line, " ")), iA);
  decode_string(std::string(std::strtok(NULL, " ")), iB);
  decode_string(std::string(std::strtok(NULL, " ")), iC);
  decode_string(std::string(std::strtok(NULL, " ")), iD);

  tr::snip Snip {};

  *Snip.D[0].position.x = Places[iA[0]][0];
  *Snip.D[0].position.y = Places[iA[0]][1];
  *Snip.D[0].position.z = Places[iA[0]][2];

  *Snip.D[1].position.x = Places[iB[0]][0];
  *Snip.D[1].position.y = Places[iB[0]][1];
  *Snip.D[1].position.z = Places[iB[0]][2];

  *Snip.D[2].position.x = Places[iC[0]][0];
  *Snip.D[2].position.y = Places[iC[0]][1];
  *Snip.D[2].position.z = Places[iC[0]][2];

  *Snip.D[3].position.x = Places[iD[0]][0];
  *Snip.D[3].position.y = Places[iD[0]][1];
  *Snip.D[3].position.z = Places[iD[0]][2];

  *Snip.D[0].normal.x = Normals[iA[2]][0];
  *Snip.D[0].normal.y = Normals[iA[2]][1];
  *Snip.D[0].normal.z = Normals[iA[2]][2];

  *Snip.D[1].normal.x = Normals[iB[2]][0];
  *Snip.D[1].normal.y = Normals[iB[2]][1];
  *Snip.D[1].normal.z = Normals[iB[2]][2];

  *Snip.D[2].normal.x = Normals[iC[2]][0];
  *Snip.D[2].normal.y = Normals[iC[2]][1];
  *Snip.D[2].normal.z = Normals[iC[2]][2];

  *Snip.D[3].normal.x = Normals[iD[2]][0];
  *Snip.D[3].normal.y = Normals[iD[2]][1];
  *Snip.D[3].normal.z = Normals[iD[2]][2];

  Area.push_front(Snip);
  return;
}

//## прием трех чисел в формате float в список нормалей
void loader_obj::get_vn(char *line)
{
  char *token = std::strtok(line, " ");
  std::array<float, 3> normal {};
  for(size_t i = 0; i < 3; i++)
  {
    normal[i] = std::stof(token);
    token = std::strtok(NULL, " ");
  }
  Normals.push_back(normal);
  return;
}

//## прием двух чисел в формате float в список UV текстуры
void loader_obj::get_vt(char *line)
{
  std::array<float, 2> UV {};
  UV[0] = std::stof(std::strtok(line, " "));
  UV[1] = std::stof(std::strtok(NULL, " "));
  UVs.push_back(UV);
  return;
}

//## прием трех чисел в формате float в список вершин
void loader_obj::get_v(char *line)
{
  char *token = std::strtok(line, " ");
  std::array<float, 3> Place = {Point.x, Point.y, Point.z};
  for(size_t i = 0; i < 3; i++)
  {
    Place[i] += std::stof(token);
    token = std::strtok(NULL, " ");
  }
  Places.push_back(Place);
  return;
}

//## Разбор типа переданой строки символов по ключу
void loader_obj::parsing_line(char *line)
{
  std::string LineId = std::strtok(line, " ");
  if(LineId == "v") get_v(&line[2]);
  else if(LineId == "vn") get_vn(&line[3]);
  else if(LineId == "vt") get_vt(&line[3]);
  else if(LineId == "f") get_f(&line[2]);
  return;
}

} //namespace tr
