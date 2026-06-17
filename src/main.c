#include <stdio.h>
#include "peer_discovery.h"

int main(int argc, char *argv[]) {

    /*
     * start TUI 
     */
    
    /*
     * configure UDP listening and sending to populate peers table in TUI
     */

    /*
     * when user selects a peer from the TUI, establish a TCP connection
     */

    if (discover_peers()) {
        fprintf(stderr, "discover_peers() failed\n");
        return 1;
    }

    while(1) {
        sleep(1);
        for (int i = 0; i < MAX_PEERS; i++) {
            
            pthread_mutex_lock(&peer_table_mux);
            if (strlen(peer_table[i].username) > 0)
                printf("%s, %s\n", peer_table[i].username, peer_table[i].addr);
            pthread_mutex_unlock(&peer_table_mux);
        }
    }
    return 0;
}