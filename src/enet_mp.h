#ifndef __ENET_MP_H__
#define __ENET_MP_H__


/** @file
 * ENet Mp is an utility library for ENet, which helps you create simple
 * client/server based multiplayer games.
 *
 * It manages client connections, provides synced variables and is non-obstrubtive.
 */


#include <enet.h>


/**
 * Local server instance.
 *
 * Stores connected clients, game state and handles events.
 */
typedef struct _ENetMpServer ENetMpServer;

/**
 * Connected clients as seen by the server.
 */
typedef struct _ENetMpRemoteClient ENetMpRemoteClient;

/**
 * Local client instance.
 *
 * Stores game state and handles events.
 */
typedef struct _ENetMpClient ENetMpClient;


/**
 *
 * @param host
 *
 * @param server_name
 *
 * @return
 * The server instance or `NULL` if something went wrong.
 */
ENetMpServer* enet_mp_create_server();

/**
 *
 */
void enet_mp_destroy_server( ENetMpServer* server );

/**
 * Waits for new incoming packets, sends outgoing packets and handles events.
 *
 * Should be called regulary.
 *
 * @param timeout
 */
void enet_mp_update_server( ENetMpServer* server, enet_uint32 timeout );


/**
 *
 * @param host
 *
 * @param client_name
 */
ENetMpClient* enet_mp_create_client();

/**
 *
 */
void enet_mp_destroy_client( ENetMpClient* client );

/**
 * Waits for new incoming packets, sends outgoing packets and handles events.
 *
 * Should be called regulary.
 *
 * @param timeout
 */
void enet_mp_client_service( ENetMpClient* client, enet_uint32 timeout );


#endif
