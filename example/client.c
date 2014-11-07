#include "shared.h"

int is_running = 1;

void stop()
{
    is_running = 0;
}

void print_clients( ENetMpClient* client )
{
    const int slot_count = enet_mp_client_get_client_slot_count(client);
    printf("slot_count=%d\n", slot_count);
    for(int i = 0; i < slot_count; i++)
    {
        printf("  slot=%d name=%s\n",
               i,
               enet_mp_client_get_client_name_at_slot(client, i));
    }
}

void connected( ENetMpClient* client )
{
    printf("connected: server_name=%s", enet_mp_client_get_server_name(client));
    print_clients(client);
}

void disconnected( ENetMpClient* client, ENetMpDisconnectReason reason )
{
    printf("disconnected: reason=%s", disconnect_reason_to_string(reason));
    stop();
}

void another_client_connected( ENetMpClient* client, int client_slot_index )
{
    printf("remote_client_connected: index=%d name=%s\n",
           client_slot_index,
           enet_mp_client_get_client_name_at_slot(client, client_slot_index));
}

void another_client_disconnected( ENetMpClient* client,
                                 int client_slot_index,
                                 ENetMpDisconnectReason reason )
{
    printf("remote_client_disconnected: index=%d name=%s reason=%s\n",
           client_slot_index,
           enet_mp_client_get_client_name_at_slot(client, client_slot_index),
           disconnect_reason_to_string(reason));
}

void received_packet( ENetMpClient* client,
                      int channel,
                      const ENetPacket* packet )
{
    printf("received_packet: channel=%d packet=%s",
           channel,
           packet->data);
}

int main()
{
    assert(enet_initialize() == 0);
    atexit(enet_deinitialize);

    ENetAddress server_address;
    assert(enet_address_set_host(&server_address, "localhost") == 0);
    server_address.port = PORT;

    ENetMpClientConfiguration config;
    memset(&config, 0, sizeof(config));
    config.server_address = server_address;
    config.name = "example client";
    config.callbacks.connected = connected;
    config.callbacks.disconnected = disconnected;
    config.callbacks.another_client_connected = another_client_connected;
    config.callbacks.another_client_disconnected = another_client_disconnected;
    config.callbacks.received_packet = received_packet;

    ENetMpClient* client = enet_mp_client_create(&config);
    assert(client);

    signal(SIGTERM, stop);
    signal(SIGINT, stop);
    while(is_running)
    {
        printf(".");
        fflush(stdout);
        enet_mp_client_service(client, SERVICE_TIMEOUT);
    }

    enet_mp_client_destroy(client);
    printf("\nstopped\n");
    return 0;
}
