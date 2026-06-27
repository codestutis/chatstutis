#include "chat.h"
#include "boundedBuffer.h"
#include "peer_discovery.h"
#include <pthread.h>
#include <stdlib.h>

static int socket_listen;
void *incoming_chat_buf = NULL;

void *listen_chat_conns(void *args) {
    fd_set master;
    FD_ZERO(&master);
    FD_SET(socket_listen, &master);
    int max_socket = socket_listen;

    while (1) {
        fd_set reads = master;
        if (select(max_socket + 1, &reads, NULL, NULL, NULL) < 0) {
            perror("socket() failed");
            return NULL;
        }

        for (int i = 1; i <= max_socket; i++) {
            if (!FD_ISSET(i, &reads))
                continue;

            if (i == socket_listen) {
                // new connection
                struct sockaddr_storage client_address;
                socklen_t client_len = sizeof(client_address);
                int socket_client =
                    accept(socket_listen, (struct sockaddr *)&client_address,
                           &client_len);
                if (socket_client < 0) {
                    perror("accept() failed");
                    return NULL;
                }

                FD_SET(socket_client, &master);
                if (socket_client > max_socket)
                    max_socket = socket_client;

            } else {
                // read from existing connection
                chat_packet_t *msg = malloc(sizeof(*msg));
                if (msg == NULL) {
                    perror("malloc() failed");
                    continue;
                }

                int pkt_len =
                    MAX_MESSAGE_LENGTH + MAX_USERNAME_LEN + HEADER_LEN;
                int bytes_received = recv(i, msg, pkt_len, 0);
                if (bytes_received < 1 || msg->message_type == 1) {
                    free(msg);
                    FD_CLR(i, &master);
                    close(i);
                    continue;
                }
                if (msg->version != 1) {
                    continue;
                }
                if (msg->message_type != 0) {
                    continue;
                }
                // is a chat packet and is the correct verion:

                // add message to incoming chat buffer to be processed by tui.c
                putBuffer(incoming_chat_buf, msg);
            }
        }
    }
}

int init_chat_listener() {
    incoming_chat_buf = createBuffer(64);
    if (incoming_chat_buf == NULL) {
        fprintf(stderr, "createBuffer() failed\n");
        return 1;
    }
    struct addrinfo hints;
    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_PASSIVE;

    struct addrinfo *bind_address;
    if (getaddrinfo(0, CHAT_PORT, &hints, &bind_address)) {
        perror("getaddrinfo() failed");
        return 1;
    }

    socket_listen = socket(bind_address->ai_family, bind_address->ai_socktype,
                           bind_address->ai_protocol);
    if (socket_listen < 0) {
        perror("socket() failed");
        return 1;
    }

    if (bind(socket_listen, bind_address->ai_addr, bind_address->ai_addrlen)) {
        perror("bind() failed");
        return 1;
    }
    freeaddrinfo(bind_address);

    if (listen(socket_listen, 10) < 0) {
        perror("listen() failed");
        return 1;
    }

    pthread_t listen_chats;
    if (pthread_create(&listen_chats, NULL, listen_chat_conns, NULL)) {
        perror("pthread_create() failed");
        return 1;
    }
    return 0;
}

void send_chat(peer_t *p, const char *msg, int msg_len) {
    struct addrinfo hints;
    memset(&hints, 0, sizeof(hints));
    hints.ai_socktype = SOCK_STREAM;
    struct addrinfo *peer_address;
    int status = getaddrinfo(p->addr, CHAT_PORT, &hints, &peer_address);
    if (status != 0) {
        fprintf(stderr, "getaddrinfo() failed: %s\n", gai_strerror(status));
        return;
    }

    int socket_peer;
    socket_peer = socket(peer_address->ai_family, peer_address->ai_socktype,
                         peer_address->ai_protocol);
    if (socket_peer < 0) {
        perror("socket() failed");
        return;
    }

    if (connect(socket_peer, peer_address->ai_addr, peer_address->ai_addrlen)) {
        fprintf(stderr, "connect() fail");
        return;
    }
    freeaddrinfo(peer_address);

    chat_packet_t pkt = {
        .version = 1,
        .message_type = 0,
        .from_usr = "hi",
    };
    strcpy(pkt.message_data, msg);

    // TODO: send remaining bytes if not all sent!!
    send(socket_peer, &pkt, 1058, 0);

    // TODO: better way of managing connections... new connection per message is
    // stupid
    close(socket_peer);
}
