#include <assert.h>
#include <string.h> // strlen, strncpy
#include "enet_mp.h"
#include "enet_mp_shared.h"


bool copy_string( const char* source, char* destination, int destination_size )
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

bool is_in_bounds( int index, int array_size )
{
    return index >= 0 &&
           index < array_size;
}

const char* disconnect_reason_as_string( ENetMpDisconnectReason reason )
{
    switch(reason)
    {
        case ENET_MP_DISCONNECT_UNKNOWN: return "unknown";
        case ENET_MP_DISCONNECT_MANUAL: return "manual";
        case ENET_MP_DISCONNECT_SERVER_SHUTDOWN: return "server shutdown";
        case ENET_MP_DISCONNECT_SERVER_FULL: return "server is full";
        case ENET_MP_DISCONNECT_REPLY_TIMEOUT: return "reply timeout";
        default: assert(!"Unknown reason!");
    }
}

void host_service( ENetHost* host,
                   int timeout,
                   void* context,
                   ConnectHandler connect_handler,
                   DisconnectHandler disconnect_handler,
                   ReceiveHandler receive_handler )
{
    ENetEvent event;
    int event_occured = enet_host_service(host, &event, timeout);
    assert(event_occured >= 0);
    if(event_occured > 0)
    {
        switch(event.type)
        {
            case ENET_EVENT_TYPE_CONNECT:
                printf("ENET_EVENT_TYPE_CONNECT\n");
                connect_handler(context,
                                event.peer,
                                (ConnectionType)event.data);
                break;

            case ENET_EVENT_TYPE_DISCONNECT:
                printf("ENET_EVENT_TYPE_DISCONNECT\n");
                disconnect_handler(context,
                                   event.peer,
                                   (ENetMpDisconnectReason)event.data);
                break;

            case ENET_EVENT_TYPE_RECEIVE:
                printf("ENET_EVENT_TYPE_RECEIVE\n");
                receive_handler(context,
                                event.peer,
                                event.channelID,
                                event.packet);
                enet_packet_destroy(event.packet);
                break;

            default:
                assert(!"Unknown event!");
        }
    }
    // TODO: Maybe use enet_host_check_events and enet_host_flush instead.
}

enet_uint8 get_internal_channel( InternalChannel channel, int user_channel_count )
{
    return (enet_uint8)(user_channel_count + (int)channel);
}

char* send_internal_message( ENetPeer* peer,
                             MessageType type,
                             int size,
                             int user_channel_count )
{
    const int packet_size = sizeof(MessageHeader) + size;
    ENetPacket* packet = enet_packet_create(NULL,
                                            packet_size,
                                            ENET_PACKET_FLAG_RELIABLE);

    const enet_uint8 channel = get_internal_channel(MESSAGE_CHANNEL, user_channel_count);
    const int result = enet_peer_send(peer, channel, packet);
    assert(result == 0);

    MessageHeader* header = (MessageHeader*)packet->data;
    header->type = type;

    char* data = &packet->data[sizeof(MessageHeader)];
    memset(data, 0, size);
    return data;
}

const char* read_internal_message( const ENetPacket* packet,
                                   MessageType* type,
                                   int* size )
{
    const MessageHeader* header = (const MessageHeader*)packet->data;
    *type = header->type;
    *size = packet->dataLength - sizeof(MessageHeader);
    return &packet->data[sizeof(MessageHeader)];
}
