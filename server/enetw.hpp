//
// Обертка к библиотеке enet
//
/*
// === SAMPLES

// Если клиент с уровнем "adm_key" сказал "stop", то выключить сервер
 if((memcmp(event.packet->data, CmdMap[CMD_STOP].data(),
    event.packet->dataLength) == 0 ) && (event.peer->data == &adm_key))

// Структура данных
event.packet->dataLength // длина пакета
event.packet->data       // данные
event.peer->data         // информация, установленая в блоке сверху
event.channelID          // по какому каналу

*/

#ifndef __ENETW_HPP__
#define __ENETW_HPP__

#include <iostream>
#include <cwctype>
#include <cstring>
#include <cstdlib>
#include <enet/enet.h>
#include <map>
#include <vector>

#define ERR throw std::runtime_error

namespace tr {

  enum SRV_CMD {
    CMD_ZERO,      // нуль не используется как команда
    CMD_HELLO,     // control me
    CMD_BY,        // disconnect me
    CMD_STOP,      // stop server
    CMD_RELOAD,    // reload data
    CMD_RESTART,   // restart server
    CMD_ENUM_END,  // end of the list commands
  };

  extern std::map<SRV_CMD, std::vector<unsigned char>> CmdMap;
  extern int admin_key; // ключ администратора TODO: сделать вычисляемым

  class enetw
  {
  public:
    enetw(void);
    ~enetw(void);

    ENetHost* enetw_host = nullptr;
    ENetAddress address = {};
    int n_connections = 8;        // количество подключений
    int n_channels = 0;           // max число каналов для каждого подключения
    int in_bw = 0;                // скорость приема (Кбайт/с)
    int out_bw = 0;               // скорость передачи (Кбайт/с)

    void create_server(void);
    void run_server(void);

  };

} // tr
#endif // __ENETW_HPP__
