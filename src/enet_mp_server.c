#include <assert.h>
#include <stdlib.h> // calloc, free
#include <string.h> // memset
#include "enet_mp.h"
#include "enet_mp_shared.h"


typedef enum _ClientSlotState
{
    CLIENT_SLOT_UNUSED,
    CLIENT_SLOT_CONNECTED
    // ...
} ClientSlotState;

typedef struct _ClientSlot
{
    ClientSlotState state;
    void* user_data;
    char name[MAX_NAME_SIZE];
    ENetPeer* peer;
    enet_uint32 reply_time; // Client will be disconnected if it has not
                            // replied before reply_timeout.

} ClientSlot;

struct _ENetMpServer
{
    void* user_data;
    char name[MAX_NAME_SIZE];
    ENetMpServerCallbacks callbacks;
    ENetHost* host;
    int user_channel_count;
    int client_slot_count;
    ClientSlot* client_slots;
    enet_uint32 reply_timeout;
};


ENetMpServer* enet_mp_server_create( const ENetMpServerConfiguration* config )
{
    assert(config->max_clients >= 0);
    assert(config->channel_count >= 0);

    ENetMpServer* server = (ENetMpServer*)calloc(1, sizeof(ENetMpServer));

    server->user_data = config->user_data;
    server->client_slot_count = config->max_clients;
    bool r = copy_string(config->name, server->name, sizeof(server->name));
    assert(r);
    server->callbacks = config->callbacks;
    server->user_channel_count = config->channel_count;
    server->host = enet_host_create(&config->address,
                                    server->client_slot_count,
                                    server->user_channel_count + INTERNAL_CHANNEL_COUNT,
                                    0, // unlimited ingoing bandwidth
                                    0); // unlimited outgoing bandwidth
    assert(server->host);
    server->client_slots = (ClientSlot*)calloc(server->client_slot_count,
                                               sizeof(ClientSlot));
    server->reply_timeout = 1000;

    return server;
}

static void disconnect_client_later( ENetMpServer* server, ClientSlot* slot, int reason )
{
    assert(slot->state != CLIENT_SLOT_UNUSED);
    enet_peer_disconnect_later(slot->peer, reason);
}

static void disconnect_client_now( ENetMpServer* server, ClientSlot* slot, int reason )
{
    assert(slot->state != CLIENT_SLOT_UNUSED);
    enet_peer_disconnect_now(slot->peer, reason);
    slot->state = CLIENT_SLOT_UNUSED;
}

void enet_mp_server_destroy( ENetMpServer* server )
{
    for(int i = 0; i < server->client_slot_count; i++)
    {
        ClientSlot* slot = &server->client_slots[i];
        if(slot->state != CLIENT_SLOT_UNUSED)
            disconnect_client_now(server, slot, ENET_MP_DISCONNECT_SERVER_SHUTDOWN);
    }

    enet_host_destroy(server->host);
    free(server->client_slots);
    free(server);
}

static ClientSlot* find_unused_client_slot( ENetMpServer* server )
{
    for(int i = 0; i < server->client_slot_count; i++)
    {
        ClientSlot* slot = &server->client_slots[i];
        if(slot->state == CLIENT_SLOT_UNUSED)
            return slot;
    }
    return NULL;
}

static void handle_query( const ENetMpServer* server, ENetPeer* peer )
{
    enet_peer_disconnect_now(peer, ENET_MP_DISCONNECT_UNKNOWN);
    UNIMPLEMENTED();
}

static void handle_new_client( ENetMpServer* server, ENetPeer* peer )
{
    ClientSlot* slot = find_unused_client_slot(server);
    if(slot)
    {
        memset(slot, 0, sizeof(ClientSlot));
        slot->state = CLIENT_SLOT_CONNECTED;
        slot->peer = peer;
        slot->reply_time = enet_time_get() + server->reply_timeout;
    }
    else
    {
        enet_peer_disconnect_now(peer, ENET_MP_DISCONNECT_SERVER_FULL);
        UNIMPLEMENTED();
    }
}

static void handle_unknown_connection( const ENetMpServer* server, ENetPeer* peer )
{
    enet_peer_disconnect_now(peer, ENET_MP_DISCONNECT_UNKNOWN);
    assert(!"Unknown connection type!");
}

static int find_client_slot_by_peer( const ENetMpServer* server, const ENetPeer* peer )
{
    for(int i = 0; i < server->client_slot_count; i++)
    {
        ClientSlot* slot = &server->client_slots[i];
        if(slot->peer == peer)
        {
            assert(slot->state != CLIENT_SLOT_UNUSED);
            return i;
        }
    }
    return -1;
}

static void handle_connect( void* context,
                            ENetPeer* peer,
                            ConnectionType connection_type )
{
    ENetMpServer* server = (ENetMpServer*)context;

    switch(connection_type)
    {
        case QUERY_CONNECTION:
            handle_query(server, peer);
            break;

        case CLIENT_CONNECTION:
            handle_new_client(server, peer);
            break;

        default:
            handle_unknown_connection(server, peer);
    }
}

static void handle_disconnect( void* context,
                               ENetPeer* peer,
                               ENetMpDisconnectReason reason )
{
    ENetMpServer* server = (ENetMpServer*)context;

    const int slot_index = find_client_slot_by_peer(server, peer);
    if(slot_index >= 0)
    {
        ClientSlot* slot = &server->client_slots[slot_index];
        slot->state = CLIENT_SLOT_UNUSED;
        server->callbacks.client_disconnected(server, slot_index, reason);
    }
}

static void handle_receive( void* context,
                            ENetPeer* peer,
                            int channel,
                            const ENetPacket* packet )
{
    ENetMpServer* server = (ENetMpServer*)context;
    const int slot_index = find_client_slot_by_peer(server, peer);

    const int user_channel_count = server->user_channel_count;
    if(channel < user_channel_count)
    {
        assert(slot_index >= 0);
        server->callbacks.client_sent_packet(server, slot_index, channel, packet);
    }
    else
    {
        const InternalChannel internal_channel =
            (InternalChannel)(channel-user_channel_count);

        switch(internal_channel)
        {
            case MESSAGE_CHANNEL:
                UNIMPLEMENTED();
                break;

            default:
                assert(!"Unknown internal channel!");
        }
    }
}

static void disconnect_clients_with_reply_timeout( ENetMpServer* server )
{
    for(int i = 0; i < server->client_slot_count; i++)
    {
        ClientSlot* slot = &server->client_slots[i];
        if(slot->state != CLIENT_SLOT_UNUSED &&
           slot->reply_time != 0 &&
           enet_time_get() > slot->reply_time)
        {
            disconnect_client_later(server, slot, ENET_MP_DISCONNECT_REPLY_TIMEOUT);
        }
    }
}

void enet_mp_server_service( ENetMpServer* server, int timeout )
{
    host_service(server->host, timeout, server, handle_connect,
                                                handle_disconnect,
                                                handle_receive);
    disconnect_clients_with_reply_timeout(server);
}

void* enet_mp_server_get_user_data( ENetMpServer* server )
{
    return server->user_data;
}

ENetHost* enet_mp_server_get_host( ENetMpServer* server )
{
    return server->host;
}

int enet_mp_server_get_client_slot_count( ENetMpServer* server )
{
    return server->client_slot_count;
}

static ClientSlot* get_client_slot( ENetMpServer* server, int index )
{
    assert(index >= 0);
    if(index < server->client_slot_count)
    {
        ClientSlot* slot = &server->client_slots[index];
        if(slot->state != CLIENT_SLOT_UNUSED)
            return slot;
    }
    return NULL;
}

const char* enet_mp_server_get_client_name_at_slot( ENetMpServer* server, int index )
{
    const ClientSlot* slot = get_client_slot(server, index);
    if(slot)
        return slot->name;
    else
        return NULL;
}

ENetPeer* enet_mp_server_get_client_peer_at_slot( ENetMpServer* server, int index )
{
    const ClientSlot* slot = get_client_slot(server, index);
    if(slot)
        return slot->peer;
    else
        return NULL;
}
