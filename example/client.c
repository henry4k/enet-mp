#include "shared.h"

int is_running = 1;

void stop()
{
    is_running = 0;
}

void disconnected( ENetMpClient* client, ENetMpDisconnectReason reason )
{
    printf("\ndisconnected: reason=%d\n", reason);
    stop();
}

void received_packet( ENetMpClient* client,
                      int channel,
                      const ENetPacket* packet )
{
    printf("\nreceived_packet: channel=%d packet=%s\n",
           channel,
           packet->data);
}

int main( int argc, char** argv )
{
    assert(enet_initialize() == 0);
    atexit(enet_deinitialize);

    ENetAddress server_address;
    assert(enet_address_set_host(&server_address, "localhost") == 0);
    server_address.port = PORT;

    ENetMpClientConfiguration config;
    memset(&config, 0, sizeof(config));
    config.server_address = server_address;

    const char* name = NULL;
    if(argc >= 2)
        name = argv[1];
    else
        name = "unknown";
    config.auth_data = name;
    config.auth_data_size = strlen(name)+1;

    config.callbacks.disconnected = disconnected;
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
