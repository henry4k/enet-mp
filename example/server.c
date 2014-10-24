#include "shared.h"

int is_running = 1;

void stop()
{
    is_running = 0;
}

void remote_client_connecting( ENetMpServer* server, int remote_client_index )
{
    printf("remote_client_connecting: index=%d name=%s\n",
           remote_client_index,
           enet_mp_server_get_remote_client_name(server, remote_client_index));
}

void remote_client_disconnected( ENetMpServer* server,
                                 int remote_client_index,
                                 ENetMpDisconnectReason reason )
{
    printf("remote_client_disconnected: index=%d name=%s reason=%d\n",
           remote_client_index,
           enet_mp_server_get_remote_client_name(server, remote_client_index),
           reason);
}

int main()
{
    assert(enet_initialize());
    atexit(enet_deinitialize);

    ENetAddress address;
    address.host = ENET_HOST_ANY;
    address.port = 1234;

    ENetHost* host = enet_host_create(&address, // address to bind server to
                                      32, // maximum connection count
                                      1,  // maximum channels
                                      0,  // don't throttle incoming bandwidth
                                      0); // don't throttle outgoing bandwidth
    assert(host);

    ENetMpServerConfiguration config;
    memset(&config, 0, sizeof(config));
    config.host = host;
    config.name = "example server";
    config.remote_client_connecting = remote_client_connecting;
    config.remote_client_disconnected = remote_client_disconnected;

    ENetMpServer* server = enet_mp_server_create(&config);
    assert(server);

    signal(SIGTERM, stop);
    while(is_running)
    {
        printf(".");
        enet_mp_server_service(server, SERVICE_TIMEFRAME);
    }

    enet_mp_server_destroy(server);
    enet_host_destroy(host);
    return 0;
}