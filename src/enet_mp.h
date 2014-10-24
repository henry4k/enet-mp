#ifndef __ENET_MP_H__
#define __ENET_MP_H__


/** @file
 * ENet Mp is an utility library for ENet, which helps you create simple
 * client/server based multiplayer games.
 *
 * It manages client connections, provides synced variables and is non-obstrubtive.
 */


#include <enet/enet.h>


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
     * Server name that is visible to the clients.
     */
    const char* name;

    /**
     * Callback which is triggered when a client attempts to connect.
     *
     * Is not triggered when a clients tries connecting to a full server.
     */
    void (*client_connecting)( ENetMpServer* server, int clientIndex );

    /**
     * Callback which is triggered when a client disconnected.
     */
    void (*client_disconnected)( ENetMpServer* server, int clientIndex );
} ENetMpServerCallbacks;

/**
 * Local client instance.
 *
 * Stores game state and handles events.
 */
typedef struct _ENetMpClient ENetMpClient;

/**
 * Callbacks used by the client.
 */
typedef struct _ENetMpClientCallbacks
{
    /**
     * User data pointer which can be useful in callbacks.
     */
    void* user_data;

    /**
     * The host that shall be used as client.
     */
    ENetHost* host;

    /**
     * Client name that is visible to the server and other clients.
     */
    const char* name;

    /**
     * Callback which is triggered when the client disconnected from the server.
     *
     * @param reason
     * A message
     */
    void (*disconnected)( ENetMpClient* client, const char* reason );
} ENetMpClientCallbacks;


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

int enet_mp_server_get_remote_client_count( ENetMpServer* server );

const char* enet_mp_server_get_remote_client_name( ENetMpServer* server, int index );

ENetPeer* enet_mp_server_get_remote_client_peer( ENetMpServer* server, int index );

/**
 * Queues a disconnect command for the given client.
 *
 * The command is sent and the client disconnected during the next call to
 * #enet_mp_server_service.
 *
 * @param reason
 * A message describing why the client has been disconnected.
 * May be `NULL` in which case no reason is provided to the client.
 */
void enet_mp_server_disconnect_client( ENetMpServer* server,
                                       int clientIndex,
                                       const char* reason );


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

void* enet_mp_client_get_user_data( ENetMpClient* server );

const char* enet_mp_client_get_server_name( ENetMpClient* client );

int enet_mp_client_get_remote_client_count( ENetMpClient* client );

const char* enet_mp_client_get_remote_client_name( ENetMpClient* client, int index );


#endif
