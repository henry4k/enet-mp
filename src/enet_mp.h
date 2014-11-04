#ifndef __ENET_MP_H__
#define __ENET_MP_H__


/** @file
 * ENet Mp is an utility library for ENet, which helps you create simple
 * client/server based multiplayer games.
 *
 * It manages client connections, provides synced variables and is non-obstrubtive.
 */


#include <enet/enet.h>


// ENET_MP_SHARED: Define this if you're linking enet-mp dynamically.
// ENET_MP_BUILDING_SHARED: Define this when actually building the shared library.

#define ENET_MP_SHARED // TODO

#if defined(ENET_MP_SHARED)
    #if defined(_WIN32) || defined(__CYGWIN__)
        #if defined(ENET_MP_BUILDING_SHARED)
            #if defined(__GNUC__)
                #define ENET_MP_API __attribute__ ((dllexport))
            #else
                #define ENET_MP_API __declspec(dllexport)
            #endif
        #else
            #if defined(__GNUC__)
                #define ENET_MP_API __attribute__ ((dllimport))
            #else
                #define ENET_MP_API __declspec(dllimport)
            #endif
        #endif
    #else
        #if (__GNUC__ >= 4)
            #define ENET_MP_API __attribute__ ((visibility ("default")))
        #else
            #define ENET_MP_API extern
        #endif
    #endif
#else
    #define ENET_MP_API
#endif

#ifdef __cplusplus
extern "C"
{
#endif


typedef enum _ENetMpDisconnectReason
{
    ENET_MP_DISCONNECT_MANUAL,
    ENET_MP_DISCONNECT_TIMEOUT,
    ENET_MP_DISCONNECT_FORCED_BY_SERVER
} ENetMpDisconnectReason;

/**
 * Local server instance.
 *
 * Stores connected clients, game state and handles events.
 */
typedef struct _ENetMpServer ENetMpServer;

typedef struct _ENetMpServerCallbacks
{
    /**
     * Callback which is triggered when a client attempts to connect.
     *
     * Is not triggered when a clients tries connecting to a full server.
     */
    void (*remote_client_connecting)( ENetMpServer* server, int remote_client_index );

    /**
     * Callback which is triggered when a client disconnected.
     */
    void (*remote_client_disconnected)( ENetMpServer* server,
                                        int remote_client_index,
                                        ENetMpDisconnectReason reason );

    /**
     * Callback which is triggered when the server received a packet from a client.
     *
     * The packet is destroyed after this call, so you don't need to destroy it
     * yourself.
     */
    void (*remote_client_sent_packet)( ENetMpServer* server,
                                       int remote_client_index,
                                       int channel,
                                       ENetPacket* packet );

} ENetMpServerCallbacks;

/**
 * Configuration used to create a server instance.
 *
 * To be future proof, this structure should be zeroed with `memset` before use.
 */
typedef struct _ENetMpServerConfiguration
{
    /**
     * User data pointer which can be useful in callbacks.
     */
    void* user_data;

    /**
     * Address under which the server will be available.
     */
    ENetAddress address;

    /**
     * Amount of communication channels ENet shall provide.
     */
    int channel_count;

    /**
     * Maximum clients that may be simultaneously connected.
     */
    int max_clients;

    /**
     * Server name that is visible to the clients.
     */
    const char* name;

    ENetMpServerCallbacks callbacks;

} ENetMpServerConfiguration;

/**
 * Local client instance.
 *
 * Stores game state and handles events.
 */
typedef struct _ENetMpClient ENetMpClient;

typedef struct _ENetMpClientCallbacks
{
    /**
     * Callback which is triggered when the client disconnected from the server.
     */
    void (*disconnected)( ENetMpClient* client, ENetMpDisconnectReason reason );

    /**
     * Callback which is triggered when the client received a packet from the server.
     *
     * The packet is destroyed after this call, so you don't need to destroy it
     * yourself.
     */
    void (*received_packet)( ENetMpClient* client, int channel, ENetPacket* packet );

    /**
     * Callback which is triggered when another client connected to the server.
     */
    void (*remote_client_connected)( ENetMpClient* client, int remote_client_index );

    /**
     * Callback which is triggered when another client disconnected.
     */
    void (*remote_client_disconnected)( ENetMpClient* client,
                                        int remote_client_index,
                                        ENetMpDisconnectReason reason );

} ENetMpClientCallbacks;

/**
 * Callbacks used by the client.
 */
typedef struct _ENetMpClientConfiguration
{
    /**
     * User data pointer which can be useful in callbacks.
     */
    void* user_data;

    /**
     * Address of the server you want to connect to.
     */
    ENetAddress server_address;

    /**
     * Amount of communication channels ENet shall provide.
     */
    int channel_count;

    /**
     * Client name that is visible to the server and other clients.
     */
    const char* name;

    ENetMpClientCallbacks callbacks;

} ENetMpClientConfiguration;


/* ---- Server ---- */

/**
 * Creates the server.
 *
 * @param configuration
 * Pointer will only be used during server creation.
 *
 * @return
 * The server instance or `NULL` if something went wrong.
 */
ENET_MP_API ENetMpServer* enet_mp_server_create( const ENetMpServerConfiguration* configuration );

ENET_MP_API void enet_mp_server_destroy( ENetMpServer* server );

/**
 * Waits for new incoming packets, sends outgoing packets and handles events.
 *
 * Should be called regulary.
 *
 * @param timeout
 * Number of milliseconds that ENet should wait for events.
 */
ENET_MP_API void enet_mp_server_service( ENetMpServer* server, int timeout );

ENET_MP_API void* enet_mp_server_get_user_data( ENetMpServer* server );

ENET_MP_API ENetHost* enet_mp_server_get_host( ENetMpServer* server );

ENET_MP_API int enet_mp_server_max_remote_clients( ENetMpServer* server );

ENET_MP_API const char* enet_mp_server_get_remote_client_name( ENetMpServer* server, int index );

ENET_MP_API ENetPeer* enet_mp_server_get_remote_client_peer( ENetMpServer* server, int index );


/* ---- Client ---- */

/**
 * Creates the client and connects to the given server address.
 *
 * @param configuration
 * Pointer will only be used during client creation.
 *
 * @return
 * The server instance or `NULL` if something went wrong.
 */
ENET_MP_API ENetMpClient* enet_mp_client_create( const ENetMpClientConfiguration* configuration );

ENET_MP_API void enet_mp_client_destroy( ENetMpClient* client );

/**
 * Waits for new incoming packets, sends outgoing packets and handles events.
 *
 * Should be called regulary.
 *
 * @param timeout
 * Number of milliseconds that ENet should wait for events.
 */
ENET_MP_API void enet_mp_client_service( ENetMpClient* client, int timeout );

ENET_MP_API void* enet_mp_client_get_user_data( ENetMpClient* client );

ENET_MP_API ENetHost* enet_mp_client_get_host( ENetMpClient* client );

ENET_MP_API const char* enet_mp_client_get_server_name( ENetMpClient* client );

ENET_MP_API ENetPeer* enet_mp_client_get_server_peer( ENetMpClient* client );

ENET_MP_API int enet_mp_client_max_remote_clients( ENetMpClient* client );

ENET_MP_API const char* enet_mp_client_get_remote_client_name( ENetMpClient* client, int index );


#ifdef __cplusplus
}
#endif

#endif
