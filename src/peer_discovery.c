#include "peer_discovery.h"
#include <netdb.h>
#include <pthread.h>
#include <string.h>
#include <sys/socket.h>

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
        if (n != sizeof(pkt)) continue;
        char sender_ip[INET_ADDRSTRLEN];
        inet_ntop(AF_INET, &src.sin_addr, sender_ip, sizeof(sender_ip));

        if (pkt.message_type == 0) {
            printf("discovered peer: %s (%s)\n", pkt.username, sender_ip);
        } else if (pkt.message_type == 1) {
            printf("peer disconnected: %s (%s)\n", pkt.username, sender_ip);
        }
    }
    return NULL;
}

int discover_peers() {
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
