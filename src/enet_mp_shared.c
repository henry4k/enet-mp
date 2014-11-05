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
