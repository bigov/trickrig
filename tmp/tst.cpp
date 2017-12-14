#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <cstring>
#include <cstdint>
//#include <glm/glm.hpp>
#include "../.extlibs/glm/glm/glm.hpp"
#include <vector>

int main()
{
  // максимальная длина строки с данными
  const static size_t max_row_len = 64;
  char id[4] = {'\0'};
  char row[max_row_len] = {'\0'};
  
  std::vector<glm::vec3> vertices {};
  std::vector<glm::vec3> normals {};
  std::vector<glm::vec2> textures {};
  std::vector<float> data {};

  glm::vec3 var {};

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
    sscanf(row, "%s %f %f %f", id, &var.x, &var.y, &var.z );
    if(std::strcmp( id, "v" ) == 0) vertices.push_back(var);
    else if(std::strcmp( id, "vn" ) == 0) normals.push_back(var);
    //else if(std::strcmp( id, "vt" ) == 0) textures.push_back({var[1], var[2]});
  }
  stream.clear();
  stream.seekg(0);
  
  int c[3], n[3];
  while(stream.getline(row, max_row_len)) // построчная обработка данных
  {
    sscanf(row, "%s %d//%d %d//%d %d//%d", id, &c[0],&n[0],&c[1],&n[1],&c[2],&n[2]);
    if(std::strcmp( id, "f" ) == 0)
    {
      for(int i = 0; i < 3; i++)
      {
        data.push_back(vertices[c[i]].x);
        data.push_back(vertices[c[i]].y);
        data.push_back(vertices[c[i]].z);
        data.push_back( normals[n[i]].x);
        data.push_back( normals[n[i]].y);
        data.push_back( normals[n[i]].z);
      }

    }

  }
  for(auto v: data) std::cout << v << ", ";
  return 0;
}

