/*
 * Сервераная часть на базе библиотеки enet
 *
 */
#include "server.hpp"
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

//## Главный цикл обработки сервером событий
void tr_loop(ENetHost* srv)
{
  ENetEvent event;
  ENetPacket * packet = nullptr;
  char adm_key = 10;      // TODO: статус авторизации клиента
  bool listen_clients = true;
  int cmd[1] = { 0, };
  enet_uint8 channel = 0; // id канала для отправки пакета

  int timeout = 500; // интервал обработки событий 500 мсек = 0.5сек
  while(( enet_host_service( srv, &event, timeout ) > 0 ) || listen_clients )
  {
    timeout = 500;
    switch( event.type )
    {
      case ENET_EVENT_TYPE_CONNECT:
        event.peer->data = &adm_key; // можно сохранить информацию о клиенте
        cmd[0] = CMD_HELLO;
        packet = enet_packet_create( cmd, sizeof(cmd), ENET_PACKET_FLAG_RELIABLE );
        enet_peer_send (event.peer, channel, packet);
        enet_host_flush (srv);
        timeout = 1;
        break;                       //TODO: добавить уровни доступа
      case ENET_EVENT_TYPE_RECEIVE:
        if(event.packet->dataLength != sizeof(cmd)) break;
        memcpy(cmd, event.packet->data, event.packet->dataLength);
        if(( cmd[0] == CMD_STOP) && (event.peer->data == &adm_key))
        {
          std::cout << "Recieved CMD_STOP\n";
          listen_clients = false;
        }

        // ответ клиенту
        cmd[0] = CMD_BY;
        packet = enet_packet_create( cmd, sizeof(cmd), ENET_PACKET_FLAG_RELIABLE );
        enet_peer_send (event.peer, channel, packet);
        enet_host_flush (srv);

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

//## Enter point
int main()
{
  std::cout << "\nServer started.\n";

  if( 0 != enet_initialize() )
  {
    std::cout << "An error occurred while initializing ENet.\n";
    return EXIT_FAILURE;
  } else {
    std::atexit( enet_deinitialize );
  }

  ENetHost* tr_server = nullptr;
  ENetAddress address = {};
  address.host = ENET_HOST_ANY; // адрес для приема пакетов
  address.port = 12888;         // какой порт слушать
  int n_connections = 8;        // количество подключений
  int n_channels = 0;           // max число каналов для каждого подключения
  int in_bw = 0;                // скорость приема (Кбайт/с)
  int out_bw = 0;               // скорость передачи (Кбайт/с)

  tr_server = enet_host_create(
    &address, n_connections, n_channels, in_bw, out_bw );

  if (nullptr == tr_server)
  {
    std::cout << "An error on creating an ENet server host.\n";
    exit( EXIT_FAILURE );
  } else {
    tr_loop( tr_server );
  }

  // job end
  enet_host_destroy( tr_server );

  std::cout << "Server closed.\n";
  return EXIT_SUCCESS;
}
