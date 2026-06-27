#include "chat.h"
#include "peer_discovery.h"
#include "tui.h"
#include <stdio.h>

char username[32];

int main(int argc, char *argv[]) {
    if (argc != 2) {
        fprintf(stderr,
                "wrong number of arguments\nusage: ./chat <username>\n");
        return 1;
    }
    if (strlen(argv[1]) >= sizeof(username)) {
        fprintf(stderr, "username too long (max 31 chars)\n");
        return 1;
    }
    memset(username, 0, sizeof(username));
    strcpy(username, argv[1]);

    /*
     * configure UDP listening and sending to populate peers table in TUI
     */
    if (discover_peers()) {
        fprintf(stderr, "discover_peers() failed\n");
        return 1;
    }

    /*
     * listen for TCP connections for chats
     */
    if (init_chat_listener()) {
        fprintf(stderr, "init_chat_listener() failed\n");
        return 1;
    }

    /*
     * start TUI
     */
    init_tui();

    return 0;
}
