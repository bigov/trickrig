#ifndef __SERVER_HPP__
#define __SERVER_HPP__

#include <iostream>
#include <cwctype>
#include <cstring>
#include <cstdlib>
#include <enet/enet.h>
#include <map>
#include <vector>

enum SRV_CMD {
  CMD_ZERO,      // нуль не используется как команда
  CMD_HELLO,     // control me
  CMD_BY,        // disconnect me
  CMD_STOP,      // stop server
  CMD_RELOAD,    // reload data
  CMD_RESTART,   // restart server
  CMD_ENUM_END,  // end of the list commands
};

std::map<SRV_CMD, std::vector<unsigned char>> CmdMap = {
  {CMD_HELLO, {'h','e','l','l','o'}},
  {CMD_BY, {'b','y'}},
  {CMD_STOP, {'s','t','o','p'}},
  {CMD_RELOAD, {'r','e','l','o','a','d'}},
  {CMD_RESTART, {'r','e','s','t','a','r','t'}},
};

#endif // SERVER_HPP
