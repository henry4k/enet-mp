#include <string.h> // strcmp
#include "shared.h"

int is_running = 1;

void stop()
{
    is_running = 0;
}

void client_connecting( ENetMpServer* server,
                        int client_slot,
                        const void* auth_data,
                        int auth_data_size )
{
    const char* name = (const char*)auth_data;
    printf("\nclient_connecting: slot=%d name=%s\n", client_slot, name);
    if(!name || strncmp("NOPE", name, auth_data_size) == 0)
        enet_mp_server_disconnect_client(server, client_slot, ENET_MP_DISCONNECT_AUTH_FAILURE);
}

void client_disconnected( ENetMpServer* server,
                          int client_slot,
                          ENetMpDisconnectReason reason )
{
    printf("\nclient_disconnected: slot=%d reason=%d\n",
           client_slot,
           reason);
}

void client_sent_packet( ENetMpServer* server,
                         int client_slot,
                         int channel,
                         const ENetPacket* packet )
{
    printf("\nclient_sent_packet: slot=%d channel=%d packet=%s\n",
           client_slot,
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
