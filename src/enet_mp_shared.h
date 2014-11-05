#ifndef __ENET_MP_SHARED_H__
#define __ENET_MP_SHARED_H__

#include <stdio.h> // DEBGUG
#include <stdbool.h>


#define UNIMPLEMENTED() assert(!"UNIMPLEMENTED!")

static const int MAX_NAME_SIZE = 64;

enum ConnectionType
{
    QUERY_CONNECTION,
    CLIENT_CONNECTION
};

typedef void (*EventHandler)( void* context, const ENetEvent* event );


bool copy_string( const char* source, char* destination, int destination_size );
void host_service( ENetHost* host,
                   int timeout,
                   EventHandler event_handler,
                   void* context );

#endif
