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
