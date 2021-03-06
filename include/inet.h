/*
* This source file is part of praetor, a free and open-source IRC bot,
* designed to be robust, portable, and easily extensible.
*
* Copyright (c) 2015-2017 David Zero
* All rights reserved.
*
* The following code is licensed for use, modification, and redistribution
* according to the terms of the Revised BSD License. The text of this license
* can be found in the "LICENSE" file bundled with this source distribution.
*/

#ifndef PRAETOR_INET
#define PRAETOR_INET

#include <tls.h>

/**
 * Performs DNS lookup for the host configured for the given network, and
 * stores the resulting list of struct addrinfo in the \c addr field.
 *
 * \param network A string indexing a struct networkinfo in the rc_network hash
 *        table.
 *
 * \return 0 on success.
 * \return -1 on failure.
 */
int inet_getaddrinfo(const char* network);

/**
 * Establishes a socket connection using the host:service combination
 * configured for the given network.
 *
 * For new networks, this function: 
 *  - Populates \c addr with the list of addrinfo structs returned by a DNS lookup
 *  - Allocates a send queue, and points \c send_queue to it.
 *  - Alocates a receive queue, points \c recv_queue to it, and sets \c
 *    recv_queue_size to the size of the receive queue.
 *
 * On success, this function stores the resultant socket file descriptor in the
 * \c sock field for the given network, and also adds it to the global monitor
 * list.
 *
 * On connection failure (and not any other failure reason), increments the
 * network's \c addr_idx counter to point to the next address to be tried. If
 * this function fails to connect using the last valid address in the list, it
 * fail immediately on every subsequent invocation until the \c addr field for
 * the given network is reset to 0.
 *
 * \param network A string indexing a struct networkinfo in the rc_network hash
 *                table.
 *
 * \return On immediate success, returns 0.
 * \return If the connection could not be established immediately, returns 1.
 * \return -1 on failure to connect using the address currently pointed to by
 *         the network's \c addr_idx field.
 * \return -2 on failure to connect with any address.
 */
int inet_connect(const char* network);

/**
 * Calls \c inet_connect() for every network present in \c rc_network. This
 * function does not fail unless a list of keys cannot be generated for
 * \rc_network; i.e, the system is out of memory.
 *
 * \return 0 on success.
 * \return -1 on failure.
 */
int inet_connect_all();

/**
 * Verifies that a non-blocking connect() to the given network completed
 * successfully. On success, upgrades the connection to a TLS connection (if
 * necessary) and adds the socket to the global monitor list. On failure,
 * increments \c addr_idx and retries the connection.
 *
 * \param network A string indexing a struct networkinfo in the rc_network hash
 *                table.
 *
 * \return On connection success, return 0.
 * \return If the connection failed, returns -1.
 */
int inet_check_connection(const char* network);

/**
 * For the given network, this function:
 *  - Removes its socket from the global monitor list
 *  - Removes its \c rc_network_sock mapping
 *  - De-allocates its send and receive queues.
 *
 * \param network A string indexing a struct networkinfo in the rc_network hash
 *                table.
 *
 * \return 0 on success.
 * \return -1 if no configuration could be found for the given network.
 */
int inet_disconnect(const char* network);

/**
 * Upgrades an already-established socket connection to a TLS connection. The
 * hostname associated with the given network will be used to verify the remote
 * host's certificate against the local certificate store. It is not necessary
 * to check for whether or not the given network is configured for TLS before
 * calling this function; it may be called after any connection.
 *
 * On success, stores the resultant libtls context in the \c ctx field of the
 * given network.
 *
 * On failure, closes the socket connection.
 *
 * \param network A string indexing a struct networkinfo in the rc_network hash
 *                table.
 *
 * \return On success, or if the given network was not configured for TLS,
 *         returns 0.
 * \return On failure, returns -1.
 */
int inet_tls_upgrade(const char* network);

/**
 * Reads from the socket belonging to the given network until its receive queue
 * is full, or until reading would block.
 *
 * If reading fails due to a connection issue, this function initiates a
 * restart of the connection via inet_disconnect() and inet_connect().
 *
 * \param network A string indexing a struct networkinfo in the rc_network hash
 *                table.
 *
 * \return -1 on failure to read from the given socket.
 * \return On success, returns 0.
 */
int inet_recv(const char* network);

/**
 * Sends a message on the socket belonging to the given network.
 *
 * If the message could not be sent for any reason, it is discarded.
 *
 * If the message could not be sent due to a connection issue, this function
 * initiates a restart of the connection via inet_disconnect() and
 * inet_connect().
 *
 * \param network A string indexing a struct networkinfo in the rc_network hash
 *                table.
 * \param buf     A buffer containing the message to send.
 * \param len     The number of characters from \c buf to send.
 *
 * \return -1 on failure to send via the given socket.
 * \return 0 on success.
 */
int inet_send_immediate(const char* network, const char* buf, size_t len);

/**
 * Attempts to send all messages currently in the given network's send queue.
 *
 * This function calls inet_send_immediate() in order to send each message.
 *
 * \param network A string indexing a struct networkinfo in the rc_network hash
 *                table.
 *
 * \return 0 on success, or if the system was out of memory.
 * \return -1 on failure to send one or more messages.
 */
int inet_send(const char* network);

/**
 * Calls inet_send() for every network that is actively connected.
 *
 * This function only fails if the system is out of memory.
 *
 * \return 0 on success.
 * \return -1 on failure.
 */
int inet_send_all();

#endif
