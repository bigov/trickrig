/*
 * 
 * Контрольная часть (клиент) на базе библиотеки enet
 *
 */
#include "enetw.hpp"

//## Enter point
int main()
{
  tr::enetw ClientEnet = {};
  ClientEnet.init_client();

  std::string srv_name = {"localhost"};
  if(!ClientEnet.connect(srv_name, 0)) return EXIT_FAILURE;

  bool ctrl = true;
  std::vector<enet_uint8> CmdEnet = {};

  while(ctrl)
  {
    CmdEnet.clear();
    std::cout << "srv: ";

    std::string n;
    std::getline(std::cin, n);
    CmdEnet.emplace_back(n[0]);
    ClientEnet.send_data(CmdEnet);
    //ClientEnet.check_events(timeout);
    if( n == "q" ) ctrl = false;
  }

  ClientEnet.disconnect(); // Закрыть соединение
  return EXIT_SUCCESS;
}
