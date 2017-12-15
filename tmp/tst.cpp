#include "loader_obj.hpp"

int main()
{
  std::string FName = "test_flat.obj";
  loader_obj Obj(FName);

  for(auto &v: Obj.Vertices)
  {
      std::cout << std::fixed << std::setprecision(3)
          << v.first[0] << ", " << v.first[1] << ", "<< v.first[2] << ", "
          << v.second[0] << ", " << v.second[1] << ", "<< v.second[2] << "\n";
  }
  return 0;
}

