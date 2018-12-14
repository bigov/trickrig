#include <string>
#include <iostream>

int main(int, char**)
{
  const std::string FontMap { u8"_`\"~!?@#$%^&*-+=(){}[]<>\\|/,.:;abcdefghijklmnopqrsЫz" }; //\
//tuvwxyzABCDEFGHIJKLMNOPQRSTUYWXYZ0123456789 абвгде\
//ёжзийклмнопрстуфхцчшщъыьэюяАБВГДЕЁЖЗИЙКЛМНОПРСТУФХ\
//ЦЧШЩЪЫЬЭЮЯ" };

    std::string x = { "z" };
    auto pos = FontMap.find(x);
    if(pos == std::string::npos) pos = 0;

    std::cout
      << ", pos = " << pos
      << ",char = " << x
      << "\n";

  return 0;
}
