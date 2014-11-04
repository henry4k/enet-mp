#include <assert.h>
#include <stdbool.h>
#include <stdlib.h> // calloc, free
#include <string.h> // memset, strlen, strncpy
#include "enet_mp.h"


#define UNIMPLEMENTED() assert(!"UNIMPLEMENTED!")

static const int MAX_NAME_SIZE = 64;


struct _ENetMpServer
{
    void* user_data;
    int max_clients;
    char name[MAX_NAME_SIZE];
    ENetMpServerCallbacks callbacks;
    ENetHost* host;
};

struct _ENetMpClient
{
    void* user_data;
    char name[MAX_NAME_SIZE];
    ENetMpClientCallbacks callbacks;
    ENetHost* host;

    char server_name[MAX_NAME_SIZE];
    ENetPeer* server_peer;
};

typedef void (*EventHandler)( const ENetEvent* event );



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

static void host_service( ENetHost* host, int timeout, EventHandler event_handler )
{
    ENetEvent event;
    int event_occured = enet_host_service(host, &event, timeout);
    assert(event_occured >= 0);
    if(event_occured > 0)
        event_handler(&event);
    // TODO: Maybe use enet_host_check_events and enet_host_flush instead.
}


/* ---- Server ---- */

ENetMpServer* enet_mp_server_create( const ENetMpServerConfiguration* config )
{
    assert(config->max_clients >= 0);

    ENetMpServer* server = calloc(1, sizeof(ENetMpServer));

    server->user_data = config->user_data;
    server->max_clients = config->max_clients;
    bool r = copy_string(config->name, server->name, sizeof(server->name));
    assert(r);
    server->callbacks = config->callbacks;
    server->host = enet_host_create(&config->address,
                                    config->max_clients,
                                    0, // unlimited channels (ENET_PROTOCOL_MAXIMUM_CHANNEL_COUNT)
                                    0, // unlimited ingoing bandwidth
                                    0); // unlimited outgoing bandwidth
    assert(server->host);

    return server;
}

void enet_mp_server_destroy( ENetMpServer* server )
{
    // TODO: Disconnect clients here!
    enet_host_destroy(server->host);
    free(server);
}

static void server_event_handler( const ENetEvent* event )
{
    UNIMPLEMENTED();
}

void enet_mp_server_service( ENetMpServer* server, int timeout )
{
    host_service(server->host, timeout, server_event_handler);
}

void* enet_mp_server_get_user_data( ENetMpServer* server )
{
    return server->user_data;
}

ENetHost* enet_mp_server_get_host( ENetMpServer* server )
{
    return server->host;
}

int enet_mp_server_get_remote_client_count( ENetMpServer* server )
{
    UNIMPLEMENTED();
    return 0;
}

const char* enet_mp_server_get_remote_client_name( ENetMpServer* server, int index )
{
    UNIMPLEMENTED();
    return NULL;
}

ENetPeer* enet_mp_server_get_remote_client_peer( ENetMpServer* server, int index )
{
    UNIMPLEMENTED();
    return NULL;
}


/* ---- Client ---- */

ENetMpClient* enet_mp_client_create( const ENetMpClientConfiguration* config )
{
    ENetMpClient* client = calloc(1, sizeof(ENetMpClient));

    client->user_data = config->user_data;
    bool r = copy_string(config->name, client->name, sizeof(client->name));
    assert(r);
    client->callbacks = config->callbacks;
    client->host = enet_host_create(NULL, // do not bind the host to an address
                                    1, // at most one connection (the server)
                                    0, // unlimited channels (ENET_PROTOCOL_MAXIMUM_CHANNEL_COUNT)
                                    0, // unlimited ingoing bandwidth
                                    0); // unlimited outgoing bandwidth
    assert(client->host);

    // TODO: Connect to the server here!

    return client;
}

void enet_mp_client_destroy( ENetMpClient* client )
{
    // TODO: Disconnect from server here!
    enet_host_destroy(client->host);
    free(client);
}

static void client_event_handler( const ENetEvent* event )
{
    UNIMPLEMENTED();
}

void enet_mp_client_service( ENetMpClient* client, int timeout )
{
    host_service(client->host, timeout, client_event_handler);
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

int enet_mp_client_get_remote_client_count( ENetMpClient* client )
{
    UNIMPLEMENTED();
    return 0;
}

const char* enet_mp_client_get_remote_client_name( ENetMpClient* client, int index )
{
    UNIMPLEMENTED();
    return NULL;
}
