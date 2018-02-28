/*
 * Сервераная часть на базе библиотеки enet
 *
 */
#include "enetw.hpp"

//## Enter point
int main()
{
  tr::enetw ServerEnet = {};
  ServerEnet.run_server();

  return EXIT_SUCCESS;
}
