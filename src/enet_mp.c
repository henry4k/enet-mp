#include <assert.h>
#include <stdbool.h>
#include <stdio.h> // DEBGUG
#include <stdlib.h> // calloc, free
#include <string.h> // memset, strlen, strncpy
#include "enet_mp.h"


#define UNIMPLEMENTED() assert(!"UNIMPLEMENTED!")

static const int MAX_NAME_SIZE = 64;

enum ConnectionType
{
    QUERY_CONNECTION,
    CLIENT_CONNECTION
};


typedef struct _ServerRemoteClient
{
    bool active;
    void* user_data;
    char name[MAX_NAME_SIZE];
    ENetPeer* peer;

} ServerRemoteClient;

typedef struct _ClientRemoteClient
{
    bool active;
    void* user_data;
    char name[MAX_NAME_SIZE];

} ClientRemoteClient;

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

typedef void (*EventHandler)( void* context, const ENetEvent* event );



/* ---- Tools ---- */

static bool copy_string( const char* source, char* destination, int destination_size )
{
    assert(source && destination && destination_size > 0);
    strncpy(destination, source, destination_size);
    if(destination[destination_size-1] == '\0')
    {
        return true;
    }
    else
    {
        destination[destination_size-1] = '\0';
        return false;
    }
}

static void host_service( ENetHost* host,
                          int timeout,
                          EventHandler event_handler,
                          void* context )
{
    ENetEvent event;
    int event_occured = enet_host_service(host, &event, timeout);
    assert(event_occured >= 0);
    if(event_occured > 0)
        event_handler(context, &event);
    // TODO: Maybe use enet_host_check_events and enet_host_flush instead.
}


/* ---- Server ---- */

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


/* ---- Client ---- */

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
