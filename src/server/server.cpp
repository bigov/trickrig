/*
 * Серверная часть на базе библиотеки enet
 *
 */
#include "enetw.hpp"

//## Enter point
int main()
{
  tr::enetw ServerEnet = {};
  std::cout << "Create complete\n";

  ServerEnet.init_server();
  std::cout << "Init complete\n";

  ServerEnet.run_server();

  return EXIT_SUCCESS;
}
