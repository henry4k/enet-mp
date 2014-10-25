#ifndef __ENET_MP_H__
#define __ENET_MP_H__


/** @file
 * ENet Mp is an utility library for ENet, which helps you create simple
 * client/server based multiplayer games.
 *
 * It manages client connections, provides synced variables and is non-obstrubtive.
 */


#include <enet/enet.h>


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
     * Maximum clients that may be simultaneously connected.
     */
    int max_clients;

    /**
     * Server name that is visible to the clients.
     */
    const char* name;

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
} ENetMpServerConfiguration;

/**
 * Local client instance.
 *
 * Stores game state and handles events.
 */
typedef struct _ENetMpClient ENetMpClient;

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
    ENetAddress address;

    /**
     * Client name that is visible to the server and other clients.
     */
    const char* name;

    /**
     * Callback which is triggered when the client disconnected from the server.
     */
    void (*disconnected)( ENetMpClient* client, ENetMpDisconnectReason reason );

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
ENetMpServer* enet_mp_server_create( const ENetMpServerConfiguration* configuration );

void enet_mp_server_destroy( ENetMpServer* server );

/**
 * Waits for new incoming packets, sends outgoing packets and handles events.
 *
 * Should be called regulary.
 *
 * @param timeframe
 * Call will block for `timeframe` milliseconds and wait for events.
 */
void enet_mp_server_service( ENetMpServer* server, int timeframe );

void* enet_mp_server_get_user_data( ENetMpServer* server );

ENetHost* enet_mp_server_get_host( ENetMpServer* server );

int enet_mp_server_get_remote_client_count( ENetMpServer* server );

const char* enet_mp_server_get_remote_client_name( ENetMpServer* server, int index );

ENetPeer* enet_mp_server_get_remote_client_peer( ENetMpServer* server, int index );


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
ENetMpClient* enet_mp_client_create( const ENetMpClientConfiguration* configuration );

void enet_mp_client_destroy( ENetMpClient* client );

/**
 * Waits for new incoming packets, sends outgoing packets and handles events.
 *
 * Should be called regulary.
 *
 * @param timeframe
 * Call will block for `timeframe` milliseconds and wait for events.
 */
void enet_mp_client_service( ENetMpClient* client, int timeframe );

void* enet_mp_client_get_user_data( ENetMpClient* client );

ENetHost* enet_mp_client_get_host( ENetMpClient* client );

const char* enet_mp_client_get_server_name( ENetMpClient* client );

ENetPeer* enet_mp_client_get_server_peer( ENetMpClient* client );

int enet_mp_client_get_remote_client_count( ENetMpClient* client );

const char* enet_mp_client_get_remote_client_name( ENetMpClient* client, int index );


#endif
