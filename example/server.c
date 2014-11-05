#include "shared.h"

int is_running = 1;

void stop()
{
    is_running = 0;
}

void client_connecting( ENetMpServer* server, int client_slot_index )
{
    printf("client_connecting: index=%d name=%s\n",
           client_slot_index,
           enet_mp_server_get_client_name_at_slot(server, client_slot_index));
}

void client_disconnected( ENetMpServer* server,
                          int client_slot_index,
                          ENetMpDisconnectReason reason )
{
    printf("client_disconnected: index=%d name=%s reason=%d\n",
           client_slot_index,
           enet_mp_server_get_client_name_at_slot(server, client_slot_index),
           reason);
}

void client_sent_packet( ENetMpServer* server,
                         int client_slot_index,
                         int channel,
                         const ENetPacket* packet )
{
    printf("client_sent_packet: index=%d name=%s channel=%d packet=%s",
           client_slot_index,
           enet_mp_server_get_client_name_at_slot(server, client_slot_index),
           channel,
           packet->data);
}

int main()
{
    assert(enet_initialize() == 0);
    atexit(enet_deinitialize);

    ENetMpServerConfiguration config;
    memset(&config, 0, sizeof(config));
    config.address.host = ENET_HOST_ANY;
    config.address.port = PORT;
    config.max_clients = 32;
    config.name = "example server";
    config.callbacks.client_connecting = client_connecting;
    config.callbacks.client_disconnected = client_disconnected;
    config.callbacks.client_sent_packet = client_sent_packet;

    ENetMpServer* server = enet_mp_server_create(&config);
    assert(server);

    signal(SIGTERM, stop);
    signal(SIGINT, stop);
    while(is_running)
    {
        printf(".");
        fflush(stdout);
        enet_mp_server_service(server, SERVICE_TIMEOUT);
    }

    enet_mp_server_destroy(server);
    printf("\nstopped\n");
    return 0;
}
