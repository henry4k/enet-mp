#ifndef __ENET_MP_SHARED_H__
#define __ENET_MP_SHARED_H__

#include <stdio.h> // DEBGUG
#include <stdbool.h>


#define UNIMPLEMENTED() assert(!"UNIMPLEMENTED!")

static const int MAX_NAME_SIZE = 64;

typedef enum _ConnectionType
{
    QUERY_CONNECTION,
    CLIENT_CONNECTION

} ConnectionType;

typedef enum _InternalChannel
{
    MESSAGE_CHANNEL,
    INTERNAL_CHANNEL_COUNT

} InternalChannel;

typedef void (*ConnectHandler)( void* context,
                                ENetPeer* peer,
                                ConnectionType connection_type );
typedef void (*DisconnectHandler)( void* context,
                                   ENetPeer* peer,
                                   ENetMpDisconnectReason reason );
typedef void (*ReceiveHandler)( void* context,
                                ENetPeer* peer,
                                int channel,
                                const ENetPacket* packet );

typedef enum _MessageType
{
    SERVER_INFORMATION_MESSAGE,
    CLIENT_LOGIN_REQUEST_MESSAGE,
    SERVER_LOGIN_RESPONSE_MESSAGE

} MessageType;

typedef struct _ServerInformationMessage
{
    char name[MAX_NAME_SIZE];
    int free_client_slots;
    int used_client_slots;

} ServerInformationMessage;

typedef struct _ClientLoginRequestMessage
{
    char name[MAX_NAME_SIZE];

} ClientLoginRequestMessage;

typedef struct _ServerLoginResponseMessage
{
    char name[MAX_NAME_SIZE];
    int max_clients;

} ServerLoginResponseMessage;


bool copy_string( const char* source, char* destination, int destination_size );

void host_service( ENetHost* host,
                   int timeout,
                   void* context,
                   ConnectHandler connect_handler,
                   DisconnectHandler disconnect_handler,
                   ReceiveHandler receive_handler );

/*
bool read_message_packet( const ENetPacket* packet,
                          MessageType* message_type,
                          Message* message );

ENetPacket* create_message_packet( MessageType type, const void* message );
*/

#endif
