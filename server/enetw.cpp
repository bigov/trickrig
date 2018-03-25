//
// Обертка к библиотеке enet
//
#include "enetw.hpp"

namespace tr {

  //## DEBUG
  void log(const char* text)
  {
    std::cout << text << "\n";
    return;
  }

  std::map<SRV_CMD, std::vector<char>> CmdMap = {
    {CMD_HELLO, {'h','e','l','l','o'}},
    {CMD_BY, {'b','y'}},
    {CMD_STOP, {'s','t','o','p'}},
    {CMD_RELOAD, {'r','e','l','o','a','d'}},
    {CMD_RESTART, {'r','e','s','t','a','r','t'}},
  };

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
    enet_host_destroy( nethost );
    enet_deinitialize();
    std::cout << "Host was shootdown.\n";
    return;
  }

  //## Create ENet host as network server
  void enetw::init_server(void)
  {
    nethost = enet_host_create( &address, srv_conns, srv_channels, in_bw, out_bw );
    if(nullptr == nethost) ERR("An error on creating an ENet server host");
    listen_clients = true;
    return;
  }

  //## Create ENet client
  void enetw::init_client(void)
  {
    nethost = enet_host_create( nullptr,
      cl_conns, cl_channels, cl_in_bw, cl_out_bw );
    if(nullptr == nethost) ERR("An error on creating an ENet client host");
    return;
  }

  //## установить соединение с сервером
  //
  // std::string& srv_name - имя хоста
  // enet_uint32 cl_data   - число, принимаемое сервером как "event.data"
  //
  bool enetw::connect(std::string& srv_name, enet_uint32 cl_data)
  {
    ENetEvent event = {};
    enet_address_set_host( &address, srv_name.c_str() );

    cl_peer = enet_host_connect( nethost, &address, cl_channels, cl_data );
    if( nullptr == cl_peer ) ERR( "Can't create peer to the " + srv_name);

    // ожидание подтверждения c сервера
    if(( enet_host_service( nethost, &event, 1000 ) > 0 )
      && ( event.type == ENET_EVENT_TYPE_CONNECT ))
    {
      return true;
    } else {
      enet_peer_reset( cl_peer );
      cl_peer = nullptr;
    }
    return false;
  }

  //## Отключиться от сервера
  void enetw::disconnect(void)
  {
    ENetEvent event;
    enet_peer_disconnect( cl_peer, 0);
    int timeout = 200;
    while( enet_host_service( nethost, &event, timeout ) > 0 )
    {
      switch( event.type )
      {
        case ENET_EVENT_TYPE_RECEIVE:
          // все входящие пакеты отбрасываем
          enet_packet_destroy( event.packet );
          break;
        case ENET_EVENT_TYPE_DISCONNECT:
          return;
        default:
          break;
      }
    }
    // если подтверждение не было получено, то соединение сбрасывается
    enet_peer_reset( cl_peer );
    tr::log("Connection was reset by timeout.\n");
    return;
  }

  //## Передача команды из клиентского соединения
  void enetw::send_data(std::vector<enet_uint8>& PostData )
  {
    send_by_peer(cl_peer, PostData);
    return;
  }

  //## Передача команды в указаный peer
  void enetw::send_by_peer(ENetPeer * peer, std::vector<enet_uint8>& PostData )
  {
    enet_uint8 channel = 0; // id канала для отправки пакета

    ENetPacket * packet = enet_packet_create(
      PostData.data(), PostData.size(), ENET_PACKET_FLAG_RELIABLE );

    enet_peer_send (peer, channel, packet);
    enet_host_flush (nethost);

    return;
  }

  //## Обработчик подключения
  void enetw::ev_connect(ENetPeer * peer)
  {
    std::string text = "connected on " + std::to_string(peer->address.port) +
      std::string(" from ") + std::to_string(peer->host->address.host);
    tr::log(text.c_str());
    return;
  }

  //## Обработчик приема данных
  //
  // Данные приходят массивами со структурой "enet_uint8" = "unsigned char"
  // Длина принятого массива передается в переменной ENetPacket->dataLength
  //
  // Можно скопировать принятый массив при помощи:
  //
  //    memcpy(TARGET, received->data, received->dataLength);
  //
  void enetw::ev_receive(ENetPeer * peer, ENetPacket * received)
  {
    std::vector<enet_uint8> ReceivedData = {};
    if(received->dataLength > 0)
    {
      ReceivedData.resize(received->dataLength);
      memcpy(ReceivedData.data(), received->data, received->dataLength);
    }
    enet_packet_destroy( received ); // после обработки пакет следует удалить

    if( ReceivedData[0] == '#') log("Detect prefix of admin command");
    if( ReceivedData[0] == '$') log("Detect prefix of user command");
    std::cout << ReceivedData.data() << "\n";

    ReceivedData.empty();
    ReceivedData.emplace_back(588);
    send_by_peer(peer, ReceivedData); // ответ клиенту
    return;
  }

  //## Обработчик сообытия отключения клиента
  void enetw::ev_disconnect(ENetPeer * peer)
  {
    std::cout << peer->connectID << " is disconnected\n";
    return;
  }

  //## опрос событий
  void enetw::check_events(int timeout)
  {
    ENetEvent event;
    while( enet_host_service( nethost, &event, timeout ) > 0 )
    {
      switch( event.type )
      {
        case ENET_EVENT_TYPE_CONNECT:
          ev_connect(event.peer);
          break;
        case ENET_EVENT_TYPE_RECEIVE:
          ev_receive(event.peer, event.packet);
          break;
        case ENET_EVENT_TYPE_DISCONNECT:
          ev_disconnect(event.peer);
          break;
        case ENET_EVENT_TYPE_NONE:
          break;
      }
    }
    return;
  }

  //## Start listening input connections
  void enetw::run_server(void)
  {
    while (listen_clients)
    {
      check_events(500);
    }
    return;
  }

} //namespace tr
