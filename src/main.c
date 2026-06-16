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
    
    printf("Hello world\n");

    if (discover_peers()) {
        fprintf(stderr, "discover_peers() failed\n");
        return 1;
    }

    while(1) {
        sleep(1);
    }
    return 0;
}
