#include "chat.h"
#include "peer_discovery.h"
#include <pthread.h>

static int socket_listen;

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
                chat_packet_t msg;
                int pkt_len = MAX_MESSAGE_LENGTH + MAX_USERNAME_LEN + HEADER_LEN;
                int bytes_received = recv(i, &msg, pkt_len, 0);
                if (bytes_received < 1 || msg.message_type == 1) {
                    FD_CLR(i, &master);
                    close(i);
                    continue;
                }
                if (msg.version != 1) {
                    continue;
                }
                if (msg.message_type != 0) {
                    continue;
                }
                // is a chat packet and is the correct verion:

                // add message to incoming chat buffer to be processed by tui.c
            }
        }
    }
}

int init_chat_listener() {
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
    return 0;
}
