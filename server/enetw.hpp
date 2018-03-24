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

  //DEBUG
  extern void log(const char *);

  enum SRV_CMD {
    CMD_ZERO,      // нуль не используется как команда
    CMD_HELLO,     // control me
    CMD_BY,        // disconnect me
    CMD_STOP,      // stop server
    CMD_RELOAD,    // reload data
    CMD_RESTART,   // restart server
    CMD_ENUM_END,  // end of the list commands
  };

  extern std::map<SRV_CMD, std::vector<char>> CmdMap;
  extern int admin_key; // ключ администратора TODO: сделать вычисляемым

  class enetw
  {
  private:
    bool listen_clients = false;
    ENetHost* nethost = nullptr;
    ENetAddress address = {};

    // настройки сервера
    int srv_conns = 8;     // количество подключений
    int srv_channels = 0;  // max число каналов для каждого подключения
    int srv_in_bw = 0;     // скорость приема (Кбайт/с)
    int srv_out_bw = 0;    // скорость передачи (Кбайт/с)

    // настройки клиента
    int cl_conns = 1;      // количество подключений
    int cl_channels = 1;   // max число каналов для каждого подключения
    int cl_in_bw = 0;      // скорость приема (Кбайт/с)
    int cl_out_bw = 0;     // скорость передачи (Кбайт/с)
    ENetPeer* cl_peer = nullptr; // клиентский peer

    void ev_connect(ENetPeer*);
    void ev_disconnect(ENetPeer*);
    void ev_receive(ENetPeer*, ENetPacket*);
    void send_by_peer(ENetPeer*, std::vector<enet_uint8>&);

  public:
    enetw(void);
    ~enetw(void);

    void init_server(void);
    void init_client(void);
    bool connect(std::string&, enet_uint32);
    void disconnect(void);
    void run_server(void);
    void send_data(std::vector<enet_uint8>&);
    void check_events(int timeout);

  };

} // tr
#endif // __ENETW_HPP__
