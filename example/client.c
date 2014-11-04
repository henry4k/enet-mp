#include "shared.h"

int is_running = 1;

void stop()
{
    is_running = 0;
}

void disconnected( ENetMpClient* client, ENetMpDisconnectReason reason )
{
    printf("disconnected: reason=%d", reason);
}

void remote_client_connected( ENetMpClient* client, int remote_client_index )
{
    printf("remote_client_connected: index=%d name=%s\n",
           remote_client_index,
           enet_mp_client_get_remote_client_name(client, remote_client_index));
}

void remote_client_disconnected( ENetMpClient* client,
                                 int remote_client_index,
                                 ENetMpDisconnectReason reason )
{
    printf("remote_client_disconnected: index=%d name=%s reason=%d\n",
           remote_client_index,
           enet_mp_client_get_remote_client_name(client, remote_client_index),
           reason);
}

void received_packet( ENetMpClient* client,
                      int channel,
                      ENetPacket* packet )
{
    printf("received_packet: channel=%d packet=%s",
           channel,
           packet->data);
}

int main()
{
    assert(enet_initialize());
    atexit(enet_deinitialize);

    ENetAddress server_address;
    assert(enet_address_set_host(&server_address, "localhost") == 0);
    server_address.port = PORT;

    ENetMpClientConfiguration config;
    memset(&config, 0, sizeof(config));
    config.server_address = server_address;
    config.name = "example client";
    config.disconnected = disconnected;
    config.remote_client_connected = remote_client_connected;
    config.remote_client_disconnected = remote_client_disconnected;
    config.received_packet = received_packet;

    ENetMpClient* client = enet_mp_client_create(&config);
    assert(client);

    signal(SIGTERM, stop);
    while(is_running)
    {
        printf(".");
        enet_mp_client_service(client, SERVICE_TIMEOUT);
    }

    enet_mp_client_destroy(client);
    return 0;
}
