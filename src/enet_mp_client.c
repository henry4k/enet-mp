#include <assert.h>
#include <stdlib.h> // calloc, free
#include "enet_mp.h"
#include "enet_mp_shared.h"


typedef struct _ClientRemoteClient
{
    bool active;
    void* user_data;
    char name[MAX_NAME_SIZE];

} ClientRemoteClient;

struct _ENetMpClient
{
    void* user_data;
    char name[MAX_NAME_SIZE];
    ENetMpClientCallbacks callbacks;
    ENetHost* host;
    int user_channel_count;
    char server_name[MAX_NAME_SIZE];
    ENetPeer* server_peer;
    int max_clients;
    ClientRemoteClient* clients;
};


ENetMpClient* enet_mp_client_create( const ENetMpClientConfiguration* config )
{
    assert(config->channel_count >= 0);

    ENetMpClient* client = calloc(1, sizeof(ENetMpClient));

    client->user_data = config->user_data;
    bool r = copy_string(config->name, client->name, sizeof(client->name));
    assert(r);
    client->callbacks = config->callbacks;
    client->user_channel_count = config->channel_count;
    client->host = enet_host_create(NULL, // do not bind the host to an address
                                    1, // at most one connection (the server)
                                    client->user_channel_count,
                                    0, // unlimited ingoing bandwidth
                                    0); // unlimited outgoing bandwidth
    assert(client->host);

    client->server_peer = enet_host_connect(client->host,
                                            &config->server_address,
                                            client->user_channel_count,
                                            CLIENT_CONNECTION);
    assert(client->server_peer);

    return client;
}

void enet_mp_client_destroy( ENetMpClient* client )
{
    enet_peer_disconnect_now(client->server_peer, ENET_MP_DISCONNECT_MANUAL);
    enet_host_destroy(client->host);
    free(client->clients);
    free(client);
}

static void client_event_handler( void* context, const ENetEvent* event )
{
    ENetMpClient* client = (ENetMpClient*)context;

    switch(event->type)
    {
        case ENET_EVENT_TYPE_CONNECT:
            printf("ENET_EVENT_TYPE_CONNECT\n");
            assert(event->peer == client->server_peer);
            break;

        case ENET_EVENT_TYPE_DISCONNECT:
            printf("ENET_EVENT_TYPE_DISCONNECT\n");
            assert(event->peer == client->server_peer);
            break;

        case ENET_EVENT_TYPE_RECEIVE:
            printf("ENET_EVENT_TYPE_RECEIVE\n");
            break;

        default:
            assert(!"Unknown event!");
    }
}

void enet_mp_client_service( ENetMpClient* client, int timeout )
{
    host_service(client->host, timeout, client_event_handler, client);
}

void* enet_mp_client_get_user_data( ENetMpClient* client )
{
    return client->user_data;
}

ENetHost* enet_mp_client_get_host( ENetMpClient* client )
{
    return client->host;
}

const char* enet_mp_client_get_server_name( ENetMpClient* client )
{
    if(client->server_name[0] != '\0')
        return client->server_name;
    else
        return NULL;
}

ENetPeer* enet_mp_client_get_server_peer( ENetMpClient* client )
{
    return client->server_peer;
}

int enet_mp_client_max_remote_clients( ENetMpClient* client )
{
    return client->max_clients;
}

static ClientRemoteClient* client_get_remote_client( ENetMpClient* client, int index )
{
    assert(index >= 0);
    if(index < client->max_clients)
    {
        ClientRemoteClient* remote_client = &client->clients[index];
        if(remote_client->active)
            return remote_client;
    }
    return NULL;
}

const char* enet_mp_client_get_remote_client_name( ENetMpClient* client, int index )
{
    const ClientRemoteClient* remote_client = client_get_remote_client(client, index);
    if(remote_client)
        return remote_client->name;
    else
        return NULL;
}
