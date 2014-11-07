#include <assert.h>
#include <stdlib.h> // calloc, free
#include <string.h> // memset
#include "enet_mp.h"
#include "enet_mp_shared.h"


typedef struct _ClientSlot
{
    bool active;
    void* user_data;
    char name[MAX_NAME_SIZE];

} ClientSlot;

struct _ENetMpClient
{
    void* user_data;
    char name[MAX_NAME_SIZE];
    ENetMpClientCallbacks callbacks;
    ENetHost* host;
    int user_channel_count;
    char server_name[MAX_NAME_SIZE];
    ENetPeer* server_peer;
    int client_slot_count;
    ClientSlot* client_slots;
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
    free(client->client_slots);
    free(client);
}

static void send_login_request( ENetMpClient* client )
{
    const size_t size = sizeof(ClientLoginRequestMessage);
    const int flags = ENET_PACKET_FLAG_RELIABLE;
    ENetPacket* packet = enet_packet_create(NULL, size, flags);
    ClientLoginRequestMessage* loginRequest = (ClientLoginRequestMessage*)packet->data;

    memset(loginRequest, 0, size);
    loginRequest->header.type = CLIENT_LOGIN_REQUEST_MESSAGE;
    if(!copy_string(client->name, loginRequest->name, sizeof(loginRequest->name)))
        assert(!"Name too long!");

    const int message_channel = client->user_channel_count + MESSAGE_CHANNEL;
    const int r = enet_peer_send(client->server_peer, message_channel, packet);
    assert(r == 0);
}

static void handle_connect( void* context,
                            ENetPeer* peer,
                            ConnectionType connection_type )
{
    ENetMpClient* client = (ENetMpClient*)context;
    assert(peer == client->server_peer);
    send_login_request(client);
}

static void handle_disconnect( void* context,
                               ENetPeer* peer,
                               ENetMpDisconnectReason reason )
{
    ENetMpClient* client = (ENetMpClient*)context;
    assert(peer == client->server_peer);
    client->callbacks.disconnected(client, reason);
}

static void handle_receive( void* context,
                            ENetPeer* peer,
                            int channel,
                            const ENetPacket* packet )
{
    ENetMpClient* client = (ENetMpClient*)context;

    const int user_channel_count = client->user_channel_count;
    if(channel < user_channel_count)
    {
        client->callbacks.received_packet(client, channel, packet);
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

void enet_mp_client_service( ENetMpClient* client, int timeout )
{
    host_service(client->host, timeout, client, handle_connect,
                                                handle_disconnect,
                                                handle_receive);
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

int enet_mp_client_get_client_slot_count( ENetMpClient* client )
{
    return client->client_slot_count;
}

static ClientSlot* get_client_slot( ENetMpClient* client, int index )
{
    assert(index >= 0);
    if(index < client->client_slot_count)
    {
        ClientSlot* slot = &client->client_slots[index];
        if(slot->active)
            return slot;
    }
    return NULL;
}

const char* enet_mp_client_get_client_name_at_slot( ENetMpClient* client, int index )
{
    const ClientSlot* slot = get_client_slot(client, index);
    if(slot)
        return slot->name;
    else
        return NULL;
}
