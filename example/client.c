#include "shared.h"

int is_running = 1;

void stop()
{
    is_running = 0;
}

void disconnected( ENetMpClient* client, const char* reason )
{
    printf("disconnected: reason=%s", reason);
}

void remote_client_connected( ENetMpClient* client, int remote_client_index )
{
    printf("remote_client_connected: index=%d name=%s\n",
           remote_client_index,
           enet_mp_client_get_remote_client_name(server, remote_client_index));
}

void remote_client_disconnected( ENetMpClient* client,
                                 int remote_client_index,
                                 ENetMpDisconnectReason reason )
{
    printf("remote_client_disconnected: index=%d name=%s reason=%d\n",
           remote_client_index,
           enet_mp_client_get_remote_client_name(server, remote_client_index),
           reason);
}

int main()
{
    assert(enet_initialize());
    atexit(enet_deinitialize);

    ENetHost* host = enet_host_create(NULL, // no address bound
                                      1,  // we only need one connection
                                      1,  // maximum channels
                                      0,  // don't throttle incoming bandwidth
                                      0); // don't throttle outgoing bandwidth
    assert(host);

    ENetMpClientConfiguration config;
    memset(&config, 0, sizeof(config));
    config.host = host;
    config.name = "example client";
    config.disconnected = disconnected;
    config.remote_client_connected = remote_client_connected;
    config.remote_client_disconnected = remote_client_disconnected;

    ENetMpClient* client = enet_mp_client_create(&config);
    assert(client);

    signal(SIGTERM, stop);
    while(is_running)
    {
        printf(".");
        enet_mp_client_service(client, SERVICE_TIMEFRAME);
    }

    enet_mp_client_destroy(client);
    enet_host_destroy(host);
    return 0;
}