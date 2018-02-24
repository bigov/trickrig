/*
 * Сервераная часть на базе библиотеки enet
 *
 */
#include <iostream>
#include <cstdlib>
#include <enet/enet.h>

void tr_loop(ENetHost* srv)
{
  ENetEvent event;
  char client_key = 0;  //TODO: статус авторизации клиента

  bool listen_clients = true;
  int n_loops = 20;
  const char cmd_stop[] = "stop";

  /* интервал ожидания события 1 секунда (1000 мсек) */
  while(( enet_host_service( srv, &event, 1000 ) > 0 ) || listen_clients )
  {
    switch( event.type )
    {
      case ENET_EVENT_TYPE_CONNECT:
        event.peer->data = &client_key; // можно сохранить информацию о клиенте
        break;
      case ENET_EVENT_TYPE_RECEIVE:
        std::cout << "Recieved packet: "
         << event.packet->data << "\n";

        if( 0 == strncmp(reinterpret_cast<const char*>(event.packet->data),
          cmd_stop, 4)) n_loops = 0;

        /*
        event.packet->dataLength // длина пакета
        event.packet->data       // данные
        event.peer->data         // информация, установленая в блоке сверху
        event.channelID          // по какому каналу
        */
        /* после обработки пакет следует удалить */
        enet_packet_destroy( event.packet );
        break;
      case ENET_EVENT_TYPE_DISCONNECT:
        std::cout << event.peer->data << " disconnected\n";
        event.peer->data = NULL;
        break;
      case ENET_EVENT_TYPE_NONE:
        std::cout << n_loops << " loops remain\n";
        break;
    }
    if( --n_loops < 1 ) listen_clients = false;
  }

  std::cout << "No connections - exit.\n";
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

  std::cout << "Server completed.\n";
  return EXIT_SUCCESS;
}
