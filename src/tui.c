#include "tui.h"
#include "peer_discovery.h"
#include <ncurses.h>

void draw_screen() {
    clear();
    for (int i = 0; i < MAX_PEERS; i++) {
        pthread_mutex_lock(&peer_table_mux);
        if (strlen(peer_table[i].username) > 0)
            printw("%s, %s\n", peer_table[i].username, peer_table[i].addr);
        pthread_mutex_unlock(&peer_table_mux);
    }
    refresh();
}

int init_tui() {
    initscr();
    raw();
    noecho();
    keypad(stdscr, TRUE);
    timeout(100);
    curs_set(0);

    int running = 1;
    while (running) {
        int ch = getch();

        switch (ch) {
            case 'q':
                running = 0;
                break;
            case KEY_UP:
                break;
            case KEY_DOWN:
                break;
        }

        draw_screen();
    }

    endwin();
    return 0;
}
