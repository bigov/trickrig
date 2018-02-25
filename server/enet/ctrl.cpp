/*
 * 
 * Контрольная часть (клиент) на базе библиотеки enet
 *
 */
#include <iostream>
#include <cstdlib>
#include <enet/enet.h>

//## Отключиться от сервера
void tr_disconnect(ENetHost* client, ENetPeer* peer)
{
  ENetEvent event;
  enet_peer_disconnect( peer, 0);
  // ожидание подтверждения в течение 2 секунд
  while( enet_host_service( client, &event, 2000 ) > 0 )
  {
    switch( event.type )
    {
      case ENET_EVENT_TYPE_RECEIVE:
        // вывод входящего пакета
        std::cout << event.packet->data << "\n";
        enet_packet_destroy( event.packet );
        break;
      case ENET_EVENT_TYPE_DISCONNECT:
        std::cout << "Disconnect complete.\n";
        return;
      default:
        break;
    }
  }
  // если подтверждение не было получено, то соединение сбрасывается
  enet_peer_reset( peer );
  std::cout << "Connection was reset by timeout.\n";
  return;
}

//## Передача на сервер данных
void tr_ctrl(ENetHost* client, ENetPeer* peer)
{
  std::string UserCommand = {};
  enet_uint8 channel = 0;       // id канала для отправки пакета
  ENetPacket * packet = nullptr;
  bool ctrl = true;

  while(ctrl)
  {
    std::cout << "srv: ";
    UserCommand.clear();
    std::getline( std::cin, UserCommand );
    packet = enet_packet_create( UserCommand.c_str(), UserCommand.size() + 1,
      ENET_PACKET_FLAG_RELIABLE );
    enet_peer_send (peer, channel, packet);
    enet_host_flush (client);
    if( UserCommand == "exit" ) ctrl = false;
  }

  tr_disconnect( client, peer ); // Закрыть соединение
  return;
}

//## Enter point
int main()
{
  std::cout << "\nstart\n";

  // enet prepare
  if( 0 != enet_initialize() )
  {
    std::cout << "FAILURE: an error initializing ENet.\n";
    return EXIT_FAILURE;
  } else {
    std::atexit( enet_deinitialize );
  }

  ENetHost * tr_client = nullptr;
  int n_connections = 1; // количество подключений
  int n_channels = 2;    // число каналов для передачи данных
  int in_bw =  0;        // скорость приема байт/с   (128000=1Mb/c)
  int out_bw = 0;        // скорость передачи байт/с (128000=1Mb/c)

  tr_client = enet_host_create(
    nullptr, n_connections, n_channels, in_bw, out_bw);

  if( nullptr == tr_client )
  {
    std::cout << "FAILURE: an error on creating an ENet client.\n";
    exit( EXIT_FAILURE );
  }
  // job start
  ENetAddress address = {};
  enet_address_set_host( &address, "localhost" );
  address.port = 12888;
  ENetEvent event = {};

  // установить соединение с сервером
  ENetPeer* peer = nullptr;
  size_t channels_count = 1; // запрашиваемое число каналов для передачи даных
  enet_uint32 user_data = 0; // целое, видимое сервером как "event.data"
  peer = enet_host_connect( tr_client, &address, channels_count, user_data );
  if( nullptr == peer )
  {
    std::cout << "Not server on " << address.host
      << ":" << address.port << "\n";
    //exit( EXIT_FAILURE );
  } else {
    // ожидание подтверждения установления соединения в течение 5 секунд
    if(( enet_host_service( tr_client, &event, 1000 ) > 0 ) &&
       ( event.type == ENET_EVENT_TYPE_CONNECT ))
    {
      std::cout << "Connected to " << peer->address.host
        <<":" << peer->address.port << "\n";
    } else {
      std::cout << "Fail connecting - reset\n";
      enet_peer_reset( peer );   // сброс соединения
      peer = nullptr;
    }
  }

  if( nullptr != peer ) tr_ctrl( tr_client, peer ); // Цикл управления сервером
  enet_host_destroy( tr_client );   // Очиститьданные - закончить работу
  std::cout << "completed\n";
  return EXIT_SUCCESS;
}
