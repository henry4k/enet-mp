#include <assert.h>
#include <stdlib.h> // malloc, calloc, free
#include <string.h> // memcpy
#include "enet_mp.h"
#include "enet_mp_shared.h"


typedef struct _ClientSlot
{
    bool active;
    void* user_data;

} ClientSlot;

struct _ENetMpClient
{
    void* user_data;
    ENetMpClientCallbacks callbacks;
    ENetHost* host;
    int user_channel_count;
    ENetPeer* server_peer;

    char* auth_data;
    int auth_data_size;
};


ENetMpClient* enet_mp_client_create( const ENetMpClientConfiguration* config )
{
    assert(config->channel_count >= 0);

    ENetMpClient* client = calloc(1, sizeof(ENetMpClient));

    client->user_data = config->user_data;
    client->callbacks = config->callbacks;
    client->user_channel_count = config->channel_count;
    client->host = enet_host_create(NULL, // do not bind the host to an address
                                    1, // at most one connection (the server)
                                    client->user_channel_count,
                                    0, // unlimited ingoing bandwidth
                                    0); // unlimited outgoing bandwidth
    assert(client->host);

    if(config->auth_data)
    {
        assert(config->auth_data_size > 0);
        client->auth_data = malloc(config->auth_data_size);
        memcpy(client->auth_data,
               config->auth_data,
               config->auth_data_size);
        client->auth_data_size = config->auth_data_size;
    }
    else
    {
        client->auth_data = NULL;
        client->auth_data_size = 0;
    }

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
    if(client->auth_data)
        free(client->auth_data);
    free(client);
}

static void handle_connect( void* context,
                            ENetPeer* peer,
                            ConnectionType connection_type )
{
    ENetMpClient* client = (ENetMpClient*)context;
    assert(peer == client->server_peer);

    const int size = sizeof(ClientAuthRequestHeader) +
                     client->auth_data_size;
    char* data = send_internal_message(client->server_peer,
                                       CLIENT_AUTH_REQUEST_MESSAGE,
                                       size,
                                       client->user_channel_count);

    ClientAuthRequestHeader* header = (ClientAuthRequestHeader*)data;
    // No need to fill out header as its empty.

    if(client->auth_data)
    {
        assert(client->auth_data_size > 0);

        char* auth_data_target = &data[size - client->auth_data_size];
        memcpy(auth_data_target, client->auth_data, client->auth_data_size);

        free(client->auth_data);
        client->auth_data = NULL;
        client->auth_data_size = 0;
    }
}

static void handle_disconnect( void* context,
                               ENetPeer* peer,
                               ENetMpDisconnectReason reason )
{
    ENetMpClient* client = (ENetMpClient*)context;
    assert(peer == client->server_peer);
    printf("handle_disconnect: reason='%s'\n",
            disconnect_reason_as_string(reason));
    client->callbacks.disconnected(client, reason);
}

static void handle_activation_message( ENetMpClient* client,
                                       const char* data,
                                       int size )
{
    UNIMPLEMENTED();
}

static void handle_internal_message( ENetMpClient* client,
                                     const ENetPacket* packet )
{
    MessageType type;
    int size;
    const char* data = read_internal_message(packet, &type, &size);

    switch(type)
    {
        case SERVER_CLIENT_ACTIVATION_MESSAGE:
            handle_activation_message(client, data, size);
            break;

        default:
            assert(!"Unknown internal message type!");
    }
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
                handle_internal_message(client, packet);
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

ENetPeer* enet_mp_client_get_server_peer( ENetMpClient* client )
{
    return client->server_peer;
}
