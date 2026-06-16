#include <arpa/inet.h>
#include <errno.h>
#include <netdb.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

#include <stdio.h>
#include <string.h>

#define BCAST_ADDR "255.255.255.255"
#define ANNOUNCE_INTERVAL 5
#define DEAD_INTERVAL 20
#define DISCOVERY_PORT "45678"

static int discovery_sock = -1;

typedef struct {
    uint8_t version;
    uint8_t message_type;
    char username[32];
} __attribute__((packed)) peer_discover_packet;

/*
 * make a UDP socket bind to port :2345
 * send broadcast packets every 5 seconds
 * listen for packets from other peers
 */
int discover_peers();

static void *sender_thread(void *arg);

static void *receiver_thread(void *arg);