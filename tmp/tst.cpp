#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <cstring>
#include <cstdint>
#include <glm/glm.hpp>
#include <vector>

int main()
{
  // максимальная длина строки с данными
  const static size_t max_row_len = 64;
  char row_id[4] = {'\0'};
  char row[max_row_len] = {'\0'};
  glm::vec3 vertex {};

  // чтение в бинарном режиме содержимого файла полностью в память
  std::string FContent {};
  std::ifstream FStream{"test_flat.obj", std::ios::binary | std::ios::ate};
  if( FStream )
  {
    auto size = FStream.tellg();
    FContent.resize(size, '\0');
    FStream.seekg(0);
    if(!FStream.read(&FContent[0], size))
      std::cout << "Can't read file content";
  } else { std::cout << "Can't open file";}

  std::istringstream stream(FContent); // открыть как поток
  while(stream.getline(row, max_row_len)) // построчная обработка данных
  {
    sscanf(row, "%s %f %f %f", row_id, &vertex.x, &vertex.y, &vertex.z );
    if(std::strcmp( row_id, "v" ) == 0 )
      std::cout <<"vertices: " << vertex.x <<", " << vertex.y <<", " << vertex.z <<"\n";
    if(std::strcmp( row_id, "vn" ) == 0)
      std::cout <<"normals: " << vertex.x <<", " << vertex.y <<", " << vertex.z <<"\n";
  }
  // удалить комментарии вначале файла


  return 0;
}
