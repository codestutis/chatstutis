#ifndef PEER_DISCOVERY_H
#define PEER_DISCOVERY_H

#include <arpa/inet.h>
#include <errno.h>
#include <netdb.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>
#include <pthread.h>

#include <stdio.h>
#include <string.h>
#include <time.h>

#define BCAST_ADDR "255.255.255.255"
#define ANNOUNCE_INTERVAL 5
#define DEAD_INTERVAL 20
#define DISCOVERY_PORT "45678"
#define MAX_PEERS 64

typedef struct {
    char username[32];
    char addr[INET_ADDRSTRLEN];
    time_t last_seen;
} peer;

extern pthread_mutex_t peer_table_mux;
extern peer peer_table[MAX_PEERS];
extern int discovery_sock;

typedef struct {
    uint8_t version;
    uint8_t message_type;
    char username[32];
} __attribute__((packed)) peer_discover_packet;

static inline void initialize_peer_table() {
    for (int i = 0; i < MAX_PEERS; i++) {
        peer_table[i].addr[0] = '\0';
        peer_table[i].username[0] = '\0';
        peer_table[i].last_seen = 0;
    }
}

/*
 * make a UDP socket bind to port :45678
 * send broadcast packets every 5 seconds
 * listen for packets from other peers
 */
int discover_peers();

static void *sender_thread(void *arg);

static void *receiver_thread(void *arg);

#endif