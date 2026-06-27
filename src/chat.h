#ifndef CHAT_H
#define CHAT_H

#include "peer_discovery.h"
#include <pthread.h>
#include <stdint.h>
#define CHAT_PORT "56789"
#define MAX_MESSAGE_LENGTH 1024
#include <arpa/inet.h>
#include <errno.h>
#include <netdb.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

typedef struct {
    uint8_t version;
    uint8_t message_type;
    char from_usr[MAX_USERNAME_LEN];
    char message_data[MAX_MESSAGE_LENGTH];
} chat_packet_t;

extern void *incoming_chat_buf;

/*
 * setup a listen()/accept() socket on each peer for future TCP connections
 */
int init_chat_listener();

/*
 * connect to `peer_t p`
 */
void send_chat(peer_t *p, const char *, int);

#endif
