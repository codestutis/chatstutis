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
     * start TUI
     */
    init_tui();

    return 0;
}
