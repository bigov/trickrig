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

//## конструктор класса построчно считывает файл
loader_obj::loader_obj(const std::string & FName)
{
  std::string LineBuffer {}; // используем в качестве динамического буфера
  std::ifstream FStream{ FName, std::ios::binary | std::ios::ate };

  if( FStream )
  {
    auto size = FStream.tellg(); size += 1;
    LineBuffer.resize(size, '\0'); // размер буфера в размер файла
    FStream.seekg(0); size -= 1;
    while (FStream.getline(&LineBuffer[0], size)) obj_parsing_line(&LineBuffer[0]);
  } else { std::cout << "Can't open file"; }

  places.clear();
  normals.clear();
  return;
}

//## формирование блока данных для вершины
void loader_obj::obj_make_vertex(const std::string & f)
{
 /***
  * 1. Так как индексы текстурных координат отсутуствуют,
  * то используются два значения с разделителем из двух слэшей.
  * Если нужна поддержка текстур, то блок данных будет состоять
  * из трех целых, разделеных одним слэшем.
  *
  * 2. В стандарте .obj индексы фэйсов (f...) начинаются с единицы, а не с
  * нуля, как в индексации C++ массивов. Поэтому при формировании пары,
  * чтобы получить целевой индекс, уменьшаем полученное число на 1
  */

  std::string::size_type sz;
  auto idx0 = std::stoi(f, &sz);
  //auto idx1 = ... индекс текстурной карты - не используется (пока?)
  auto idx2 = std::stoi(f.substr(sz + 2));

  Vertices.push_back(std::make_pair(places[idx0-1], normals[idx2-1]));
  return;
}

//## прием трех групп индексов, образующих треугольник
void loader_obj::obj_get_f(char *line)
{
  obj_make_vertex(std::string(std::strtok(line, " ")));
  obj_make_vertex(std::string(std::strtok(NULL, " ")));
  obj_make_vertex(std::string(std::strtok(NULL, " ")));
  return;
}

//## прием трех чисел в формате float в список нормалей
void loader_obj::obj_get_vn(char *line)
{
  char *token = std::strtok(line, " ");
  std::array<float, 3> normal {};
  for(size_t i = 0; i < 3; i++)
  {
    normal[i] = std::stof(token);
    token = std::strtok(NULL, " ");
  }
  normals.push_back(normal);
  return;
}

//## прием трех чисел в формате float в список вершин
void loader_obj::obj_get_v(char *line)
{
  char *token = std::strtok(line, " ");
  std::array<float, 3> plase {};
  for(size_t i = 0; i < 3; i++)
  {
    plase[i] = std::stof(token);
    token = std::strtok(NULL, " ");
  }
  places.push_back(plase);
  return;
}

//## Разбор типа переданой строки символов по ключу
void loader_obj::obj_parsing_line(char *line)
{
  std::string LineId = std::strtok(line, " ");
  if(LineId == "v") obj_get_v(&line[2]);
  else if(LineId == "vn") obj_get_vn(&line[3]);
  else if(LineId == "f") obj_get_f(&line[2]);
  return;
}

