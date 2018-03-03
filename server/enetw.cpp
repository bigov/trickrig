//
// Обертка к библиотеке enet
//
#include "enetw.hpp"

namespace tr {

  std::map<SRV_CMD, std::vector<unsigned char>> CmdMap = {
    {CMD_HELLO, {'h','e','l','l','o'}},
    {CMD_BY, {'b','y'}},
    {CMD_STOP, {'s','t','o','p'}},
    {CMD_RELOAD, {'r','e','l','o','a','d'}},
    {CMD_RESTART, {'r','e','s','t','a','r','t'}},
  };

  int admin_key = 888; // ключ администратора TODO: сделать вычисляемым

  //## Обработчик подключения
  void enetw::ev_connect(void)
  {
    //event.peer->data - информация от клиента при подключении
    int cmd[1] = { 0, };
    enet_uint8 channel = 0; // id канала для отправки пакета

    cmd[0] = CMD_HELLO;
    ENetPacket * packet = enet_packet_create( cmd, sizeof(cmd),
      ENET_PACKET_FLAG_RELIABLE );
    enet_peer_send (event.peer, channel, packet);
    enet_host_flush (enetw_host);

    return;
  }

  //## Default constructor
  enetw::enetw(void)
  {
    if(0 != enet_initialize()) ERR("An error initializing ENet");
    address.host = ENET_HOST_ANY; // адрес для приема пакетов
    address.port = 12888;         // какой порт слушать
    return;
  }

  //## Default destructor
  enetw::~enetw(void)
  {
    enet_host_destroy( enetw_host );
    enet_deinitialize();
    std::cout << "Host was shootdown.\n";
    return;
  }

  //## Create ENet host as network server
  void enetw::create_server(void)
  {
    enetw_host = enet_host_create( &address, n_connections, n_channels, in_bw, out_bw );
    if(nullptr == enetw_host) ERR("An error on creating an ENet server host");
    return;
  }

  //## Start listening input connections
  void enetw::run_server(void)
  {
    create_server();

    ENetPacket * packet = nullptr;
    bool listen_clients = true;
    int cmd[1] = { 0, };
    enet_uint8 channel = 0; // id канала для отправки пакета

    int timeout = 500;   // интервал обработки событий 500 мсек = 0.5сек
    while(( enet_host_service( enetw_host, &event, timeout ) > 0 ) || listen_clients )
    {
      timeout = 500;
      switch( event.type )
      {
      case ENET_EVENT_TYPE_CONNECT:
        ev_connect();
        timeout = 1;
        break;                       //TODO: добавить уровни доступа
      case ENET_EVENT_TYPE_RECEIVE:
        if(event.packet->dataLength != sizeof(cmd)) break;
        memcpy(cmd, event.packet->data, event.packet->dataLength);
        if(( cmd[0] == CMD_STOP) && (event.peer->data == &tr::admin_key))
        {
          std::cout << "Recieved CMD_STOP\n";
          listen_clients = false;
        }

        // ответ клиенту
        cmd[0] = CMD_BY;
        packet = enet_packet_create( cmd, sizeof(cmd), ENET_PACKET_FLAG_RELIABLE );
        enet_peer_send (event.peer, channel, packet);
        enet_host_flush (enetw_host);

        // после обработки пакет следует удалить
        enet_packet_destroy( event.packet );
        break;
      case ENET_EVENT_TYPE_DISCONNECT:
        std::cout << event.peer->data << " disconnected\n";
        event.peer->data = NULL;
        break;
      case ENET_EVENT_TYPE_NONE:
        break;
      }
    }
    return;
  }

} //namespace tr
