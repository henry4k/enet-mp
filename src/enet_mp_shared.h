#ifndef __ENET_MP_SHARED_H__
#define __ENET_MP_SHARED_H__

#include <stdio.h> // DEBGUG
#include <stdbool.h>


#define UNIMPLEMENTED() assert(!"UNIMPLEMENTED!")

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
    CLIENT_AUTH_REQUEST_MESSAGE,
    SERVER_CLIENT_ACTIVATION_MESSAGE

} MessageType;

typedef struct _MessageHeader
{
    enet_uint8 type;

} MessageHeader;

typedef struct _ServerInformationMessage
{
    enet_uint8 free_client_slots;
    enet_uint8 used_client_slots;

} ServerInformationMessage;

typedef struct _ClientAuthRequestHeader
{
    enet_uint8 debug_padding; // TODO: Remove this later on.

} ClientAuthRequestHeader;

typedef struct _ServerAuthResponseMessage
{
    enet_uint8 max_clients;

} ServerAuthResponseMessage;


// TODO: Remove copy_string as its not used anymore.
bool copy_string( const char* source, char* destination, int destination_size );

bool is_in_bounds( int index, int array_size );

const char* disconnect_reason_as_string( ENetMpDisconnectReason reason );

void host_service( ENetHost* host,
                   int timeout,
                   void* context,
                   ConnectHandler connect_handler,
                   DisconnectHandler disconnect_handler,
                   ReceiveHandler receive_handler );

enet_uint8 get_internal_channel( InternalChannel channel, int user_channel_count );

char* send_internal_message( ENetPeer* peer,
                             MessageType type,
                             int size,
                             int user_channel_count );

const char* read_internal_message( const ENetPacket* packet,
                                   MessageType* type,
                                   int* size );


#endif
