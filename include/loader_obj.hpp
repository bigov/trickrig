/**
  * file: loader_obj.hpp
  *
  * Класс загрузчик-парсер 3D файлов формата .obj
  *
  * Результат в
  *
  *     std::forward_list<tr::snip> area {};
  *
  */

#ifndef __LOADER_OBJ_HPP__
#define __LOADER_OBJ_HPP__

#include "snip.hpp"
#include "io.hpp"

namespace tr {

class loader_obj
{
  public:
    loader_obj(const std::string &FilePathName, const f3d &P);
    std::forward_list<tr::snip> Area {};

  private:
    tr::f3d Point; // Позиция начала коодинат фигуры
    std::vector<std::array<float, 3>> Places {};  // координаты
    std::vector<std::array<float, 3>> Normals {}; // нормали
    std::vector<std::array<float, 2>> UVs {};     // текстуры
    void decode_string(const std::string &, int *);
    void get_f(char *);
    void get_vn(char *);
    void get_vt(char *);
    void get_v(char *);
    void parsing_line(char *);
};

} // namespace tr
#endif
