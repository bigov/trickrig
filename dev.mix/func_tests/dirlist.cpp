/*
 * Выводит список папок в указанном месте. Компилируется командой:
 *
 * \> c++ -std=c++17 dirlist.cpp -lstdc++fs -o d && d
 *
 */
#include <iostream>
#include <filesystem>
#include <list>

namespace fs = std::filesystem;

// Возвращает список папок в указанном месте
std::list<std::string> dir_list(const std::string &path)
{
  std::list<std::string> D {};

  for(auto& it: fs::directory_iterator(path))
    if (fs::is_directory(it))  D.push_back(it.path().string());

  return std::move(D);
}

int main(int, char**)
{
  auto Dirs = dir_list("d:\\DevCpp\\WORKSPACE");

  for(auto l: Dirs) std::cout << l << "\n";

  return 0;
}

