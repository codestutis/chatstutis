#include "peer_discovery.h"
#include <netdb.h>
#include <pthread.h>
#include <string.h>
#include <sys/socket.h>

pthread_mutex_t peer_table_mux = PTHREAD_MUTEX_INITIALIZER;
peer peer_table[MAX_PEERS];
int discovery_sock = -1;

int insert_peer(char *username, char *addr) {
    for (int i = 0; i < MAX_PEERS; i++) {
        if (strlen(peer_table[i].username) == 0) {
            strcpy(peer_table[i].username, username);
            strcpy(peer_table[i].addr, addr);
            peer_table[i].last_seen = time(NULL);
            return 0;
        }
    }
    return 1;
}

int update_peer_table(char *username, char *addr, int message_type) {
    pthread_mutex_lock(&peer_table_mux);
    time_t now = time(NULL);

    // expire peers past the DEAD_INTERVAL
    for (int i = 0; i < MAX_PEERS; i++) {
        if (strlen(peer_table[i].username) > 0) {
            if (now - peer_table[i].last_seen > DEAD_INTERVAL) {
                // Clear the entry
                memset(&peer_table[i], 0, sizeof(peer));
            }
        }
    }

    // handle the current incoming packet
    for (int i = 0; i < MAX_PEERS; i++) {
        // Look for an existing match to refresh or disconnect
        if (strlen(peer_table[i].username) > 0 &&
            strcmp(username, peer_table[i].username) == 0 &&
            strcmp(addr, peer_table[i].addr) == 0) {

            if (message_type == 1) { // Disconnect
                memset(&peer_table[i], 0, sizeof(peer));
            } else if (message_type == 0) { // Refresh
                peer_table[i].last_seen = now;
            }
            pthread_mutex_unlock(&peer_table_mux);
            return 0;
        }
    }

    // no match found, insert a new peer
    if (message_type == 0) {
        int err = insert_peer(username, addr);
        pthread_mutex_unlock(&peer_table_mux);
        return err;
    }

    pthread_mutex_unlock(&peer_table_mux);
    return 1;
}

static void *sender_thread(void *arg) {
    (void)arg;

    struct sockaddr_in dest;
    memset(&dest, 0, sizeof(dest));
    dest.sin_family = AF_INET;
    dest.sin_port = htons(45678);
    dest.sin_addr.s_addr = inet_addr(BCAST_ADDR);

    peer_discover_packet pkt;
    memset(&pkt, 0, sizeof(pkt));
    pkt.version = 1;
    pkt.message_type = 0;
    strncpy(pkt.username, "test_user", sizeof(pkt.username) - 1);

    while (1) {
        if (sendto(discovery_sock, &pkt, sizeof(pkt), 0,
                   (struct sockaddr *)&dest, sizeof(dest)) < 0) {
            perror("sendto() failed");
        }
        sleep(ANNOUNCE_INTERVAL);
    }
    return NULL;
}

static void *receiver_thread(void *arg) {
    (void)arg;

    struct sockaddr_in src;
    socklen_t src_len = sizeof(src);
    peer_discover_packet pkt;

    while (1) {
        ssize_t n = recvfrom(discovery_sock, &pkt, sizeof(pkt), 0,
                             (struct sockaddr *)&src, &src_len);
        if (n < 0) {
            perror("recvfrom() failed");
            continue;
        }
        if (n != sizeof(pkt))
            continue;
        char sender_ip[INET_ADDRSTRLEN];
        inet_ntop(AF_INET, &src.sin_addr, sender_ip, sizeof(sender_ip));

        update_peer_table(pkt.username, sender_ip, pkt.message_type);
    }
    return NULL;
}

int discover_peers() {
    initialize_peer_table();
    struct addrinfo hints;
    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_DGRAM;
    hints.ai_flags = AI_PASSIVE;

    struct addrinfo *bind_address;
    int err = getaddrinfo(NULL, DISCOVERY_PORT, &hints, &bind_address);
    if (err) {
        fprintf(stderr, "getaddrinfo() failed: %s\n", gai_strerror(err));
        return 1;
    }

    discovery_sock = socket(bind_address->ai_family, bind_address->ai_socktype,
                            bind_address->ai_protocol);
    if (discovery_sock < 0) {
        perror("socket() failed");
        return 1;
    }

    // send broadcast
    int yes = 1;
    if (setsockopt(discovery_sock, SOL_SOCKET, SO_BROADCAST, &yes,
                   sizeof(yes)) < 0) {
        perror("setsockopt() SO_BROADCAST failed");
        return 1;
    }

    // let multiple instances bind to the same port
    if (setsockopt(discovery_sock, SOL_SOCKET, SO_REUSEADDR, &yes,
                   sizeof(yes)) < 0) {
        perror("setsockopt SO_REUSEADDR failed");
        return 1;
    }

    if (bind(discovery_sock, bind_address->ai_addr, bind_address->ai_addrlen)) {
        perror("bind() failed");
        return 1;
    }
    freeaddrinfo(bind_address);

    pthread_t send, recv;
    if (pthread_create(&send, NULL, sender_thread, NULL) != 0) {
        perror("pthread_create(sender) failed");
        return 1;
    }

    if (pthread_create(&recv, NULL, receiver_thread, NULL) != 0) {
        perror("pthread_create(receiver) failed");
        return 1;
    }

    return 0;
}
