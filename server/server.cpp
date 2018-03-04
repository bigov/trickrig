/*
 * Серверная часть на базе библиотеки enet
 *
 */
#include "enetw.hpp"

//## Enter point
int main()
{
  tr::enetw ServerEnet = {};
  ServerEnet.init_server();
  ServerEnet.run_server();

  return EXIT_SUCCESS;
}
