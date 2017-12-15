/**
  * file: loader_obj.hpp
  *
  * Класс загрузчик-парсер 3D файлов формата .obj
  *
  * Результат парсера размещается в массиве "Vertices", который представляет
  * из себя список записей (std::vector) из пар массивов фиксированного размера.
  *
  * Первый элемент каждой пары - это три координаты вершины, второй - три
  * координаты нормали к этой вершине. Все числа - в формате float.
  *
  * Пример использования:
  * ---------------------

  loader_obj Obj("test_flat.obj");
  for(auto &v: Obj.Vertices)
  {
    std::cout << std::fixed << std::setprecision(3)
      << v.first[0] << ", " << v.first[1] << ", "<< v.first[2] << ", "
      << v.second[0] << ", " << v.second[1] << ", "<< v.second[2] << "\n";
  }

  *
  */

#ifndef __LOADER_OBJ_HPP__
#define __LOADER_OBJ_HPP__

#include "main.hpp"

namespace tr {

class loader_obj
{
  public:
    loader_obj(const std::string & FilePathName);
    std::vector<std::pair<
      std::array<float, 3>, // 3D координаты вершины
      std::array<float, 3>  // 3D координаты нормали
    >> Vertices {};

  private:
    std::vector<std::array<float, 3>> places {};
    std::vector<std::array<float, 3>> normals {};
    void obj_make_vertex(const std::string &);
    void obj_get_f(char *);
    void obj_get_vn(char *);
    void obj_get_v(char *);
    void obj_parsing_line(char *);
};

} // namespace tr
#endif
