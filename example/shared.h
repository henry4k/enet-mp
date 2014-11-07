#ifndef __EXAMPLE_SHARED_H__
#define __EXAMPLE_SHARED_H__

#include <assert.h>
#include <stdio.h> // printf, fflush
#include <string.h> // memset
#include <stdlib.h> // atexit
#include <signal.h>
#include <enet/enet.h>
#include <enet_mp.h>


static const int PORT = 1234;
static const int SERVICE_TIMEOUT = 1000;


static const char* disconnect_reason_to_string( ENetMpDisconnectReason reason )
{
    switch(reason)
    {
        case ENET_MP_DISCONNECT_UNKNOWN:
            return "unknown";
        case ENET_MP_DISCONNECT_MANUAL:
            return "manual";
        case ENET_MP_DISCONNECT_SERVER_SHUTDOWN:
            return "server shutdown";
        case ENET_MP_DISCONNECT_SERVER_FULL:
            return "server full";
        case ENET_MP_DISCONNECT_REPLY_TIMEOUT:
            return "reply timeout";
        default:
            return "unknown reason";
    }
}

#endif
