/*
 * Сервераная часть на базе библиотеки enet
 *
 */
#include <iostream>
#include <cwctype>
#include <cstdlib>
#include <enet/enet.h>
#include <map>
#include <vector>

enum SRV_CMD {
	CMD_STOP,
	CMD_ENUM_END,
};

std::map<SRV_CMD, std::vector<wchar_t>> CmdMap = {};

void tr_loop(ENetHost* srv)
{
  ENetEvent event;
  char adm_key = 10;              //TODO: статус авторизации клиента
  bool listen_clients = true;
  
	CmdMap.emplace(CMD_STOP, L"stop"); // команда для выключения
	                                
  /* интервал ожидания события 1 секунда (1000 мсек) */
  while(( enet_host_service( srv, &event, 1000 ) > 0 ) || listen_clients )
  {
    switch( event.type )
    {
      case ENET_EVENT_TYPE_CONNECT:
        event.peer->data = &adm_key; // можно сохранить информацию о клиенте
        break;                       //TODO: добавить уровни доступа
      
			case ENET_EVENT_TYPE_RECEIVE:
        std::cout << "Recieved packet: "
         << event.packet->data << "\n";

        // Если клиент с уровнем "adm_key" сказал "stop", то выключить сервер
				if((0 == strncmp(reinterpret_cast<const char*>(event.packet->data),
          cmd_stop, 4)) && (event.peer->data == &adm_key))
				{
					listen_clients = false;
				}

        /*
        event.packet->dataLength // длина пакета
        event.packet->data       // данные
        event.peer->data         // информация, установленая в блоке сверху
        event.channelID          // по какому каналу
        */

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

  std::cout << "Shootdown server.\n";
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
