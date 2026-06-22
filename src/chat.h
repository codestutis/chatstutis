#ifndef CHAT_H
#define CHAT_H

#include "peer_discovery.h"
#include <stdint.h>
#define CHAT_PORT "56789"
#define MAX_MESSAGE_LENGTH 1024

typedef struct {
    uint8_t version;
    uint8_t message_type;
    char from_usr[MAX_USERNAME_LEN];
    char message_data[MAX_MESSAGE_LENGTH];
} chat_packet_t;

/*
 * setup a listen()/accept() socket on each peer for future TCP connections
 */
int init_chat_listener();

/*
 * connect to `peer_t p`
 */
void connect_to_peer(peer_t p);

#endif