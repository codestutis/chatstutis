#include "peer_discovery.h"
#include "tui.h"
#include <stdio.h>

int main(int argc, char *argv[]) {
    /*
     * configure UDP listening and sending to populate peers table in TUI
     */

    if (discover_peers()) {
        fprintf(stderr, "discover_peers() failed\n");
        return 1;
    }

    /*
     * when user selects a peer from the TUI, establish a TCP connection
     */

    // while (1) {
    //     sleep(5);
    //     for (int i = 0; i < MAX_PEERS; i++) {
    //         pthread_mutex_lock(&peer_table_mux);
    //         if (strlen(peer_table[i].username) > 0)
    //             printf("%s, %s\n", peer_table[i].username,
    //             peer_table[i].addr);
    //         pthread_mutex_unlock(&peer_table_mux);
    //     }
    //     printf("\n");
    // }

    /*
     * start TUI
     */
    init_tui();

    return 0;
}
