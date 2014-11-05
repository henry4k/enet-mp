#include <assert.h>
#include <stdlib.h> // calloc, free
#include "enet_mp.h"
#include "enet_mp_shared.h"


typedef struct _ServerRemoteClient
{
    bool active;
    void* user_data;
    char name[MAX_NAME_SIZE];
    ENetPeer* peer;

} ServerRemoteClient;

struct _ENetMpServer
{
    void* user_data;
    char name[MAX_NAME_SIZE];
    ENetMpServerCallbacks callbacks;
    ENetHost* host;
    int user_channel_count;
    int max_clients;
    ServerRemoteClient* clients;
};


ENetMpServer* enet_mp_server_create( const ENetMpServerConfiguration* config )
{
    assert(config->max_clients >= 0);
    assert(config->channel_count >= 0);

    ENetMpServer* server = (ENetMpServer*)calloc(1, sizeof(ENetMpServer));

    server->user_data = config->user_data;
    server->max_clients = config->max_clients;
    bool r = copy_string(config->name, server->name, sizeof(server->name));
    assert(r);
    server->callbacks = config->callbacks;
    server->user_channel_count = config->channel_count;
    server->host = enet_host_create(&config->address,
                                    server->max_clients,
                                    server->user_channel_count,
                                    0, // unlimited ingoing bandwidth
                                    0); // unlimited outgoing bandwidth
    assert(server->host);
    server->clients = (ServerRemoteClient*)calloc(config->max_clients, sizeof(ServerRemoteClient));

    return server;
}

void enet_mp_server_destroy( ENetMpServer* server )
{
    for(int i = 0; i < server->max_clients; i++)
    {
        ServerRemoteClient* client = &server->clients[i];
        if(client->active)
            enet_peer_disconnect_now(client->peer, ENET_MP_DISCONNECT_MANUAL);
    }

    enet_host_destroy(server->host);
    free(server->clients);
    free(server);
}

static void server_event_handler( void* context, const ENetEvent* event )
{
    ENetMpServer* server = (ENetMpServer*)context;

    switch(event->type)
    {
        case ENET_EVENT_TYPE_CONNECT:
            printf("ENET_EVENT_TYPE_CONNECT\n");
            const int connectionType = event->data;
            switch(connectionType)
            {
                case QUERY_CONNECTION:
                    enet_peer_disconnect_now(event->peer,
                                             ENET_MP_DISCONNECT_FORCED_BY_SERVER);
                    UNIMPLEMENTED();
                    break;

                case CLIENT_CONNECTION:
                    break;

                default:
                    enet_peer_disconnect_now(event->peer,
                                             ENET_MP_DISCONNECT_FORCED_BY_SERVER);
                    assert(!"Unknown connection type!");
            }
            break;

        case ENET_EVENT_TYPE_DISCONNECT:
            printf("ENET_EVENT_TYPE_DISCONNECT\n");
            break;

        case ENET_EVENT_TYPE_RECEIVE:
            printf("ENET_EVENT_TYPE_RECEIVE\n");
            break;

        default:
            assert(!"Unknown event!");
    }
}

void enet_mp_server_service( ENetMpServer* server, int timeout )
{
    host_service(server->host, timeout, server_event_handler, server);
}

void* enet_mp_server_get_user_data( ENetMpServer* server )
{
    return server->user_data;
}

ENetHost* enet_mp_server_get_host( ENetMpServer* server )
{
    return server->host;
}

int enet_mp_server_max_remote_clients( ENetMpServer* server )
{
    return server->max_clients;
}

static ServerRemoteClient* server_get_remote_client( ENetMpServer* server, int index )
{
    assert(index >= 0);
    if(index < server->max_clients)
    {
        ServerRemoteClient* remote_client = &server->clients[index];
        if(remote_client->active)
            return remote_client;
    }
    return NULL;
}

const char* enet_mp_server_get_remote_client_name( ENetMpServer* server, int index )
{
    const ServerRemoteClient* remote_client = server_get_remote_client(server, index);
    if(remote_client)
        return remote_client->name;
    else
        return NULL;
}

ENetPeer* enet_mp_server_get_remote_client_peer( ENetMpServer* server, int index )
{
    const ServerRemoteClient* remote_client = server_get_remote_client(server, index);
    if(remote_client)
        return remote_client->peer;
    else
        return NULL;
}
