#include "tui.h"
#include "peer_discovery.h"
#include <ncurses.h>
#include <pthread.h>

#define KEY_ESC 27

int peer_index(peer_t *curr) {
    pthread_mutex_lock(&peer_table_mux);
    for (int i = 0; i < MAX_PEERS; i++) {
        if (strcmp(peer_table[i].username, curr->username) == 0 &&
            strcmp(peer_table[i].addr, curr->addr) == 0) {
            pthread_mutex_unlock(&peer_table_mux);
            return i;
        }
    }
    pthread_mutex_unlock(&peer_table_mux);
    return -1;
}

int next_peer(peer_t *curr) {
    int p_idx = peer_index(curr);
    p_idx++;
    if (p_idx >= MAX_PEERS) 
        return 1;
    pthread_mutex_lock(&peer_table_mux);
    for (int i = p_idx; i < MAX_PEERS; i++) {
        if (strlen(peer_table[i].username) > 0) {
            *curr = peer_table[i];
            pthread_mutex_unlock(&peer_table_mux);
            return 0;
        }
    }

    pthread_mutex_unlock(&peer_table_mux);
    return 1;
}

int prev_peer(peer_t *curr) {
    int p_idx = peer_index(curr);
    p_idx--; 
    if (p_idx < 0) 
        return 1;
    pthread_mutex_lock(&peer_table_mux);
    for (int i = p_idx; i >= 0; i--) {
        if (strlen(peer_table[i].username) > 0) {
            *curr = peer_table[i];
            pthread_mutex_unlock(&peer_table_mux);
            return 0;
        }
    }
    pthread_mutex_unlock(&peer_table_mux);
    return 1;
}

void draw_peers(WINDOW *peer_window, peer_t selected_peer) {
    wclear(peer_window);
    pthread_mutex_lock(&peer_table_mux);
    for (int i = 0; i < MAX_PEERS; i++) {
        wmove(peer_window, i + 1, 1);
        if (strlen(peer_table[i].username) > 0) {
            if (strcmp(peer_table[i].username, selected_peer.username) == 0 &&
                strcmp(peer_table[i].addr, selected_peer.addr) == 0) {
                wattron(peer_window, A_BOLD | A_UNDERLINE);

                wprintw(peer_window, "%s, %s\n", peer_table[i].username,
                        peer_table[i].addr);

                wattroff(peer_window, A_BOLD | A_UNDERLINE);
            } else {
                wprintw(peer_window, "%s, %s\n", peer_table[i].username,
                        peer_table[i].addr);
            }
        }
        pthread_mutex_unlock(&peer_table_mux);
    }
    box(peer_window, 0, 0);
    wrefresh(peer_window);
}

int init_tui() {
    initscr();
    raw();
    noecho();
    keypad(stdscr, TRUE);
    timeout(100);
    curs_set(0);
    set_escdelay(50);

    // this is probably unsafe
    peer_t selected_peer = {0};
    selected_peer = peer_table[0];

    int row, col;
    getmaxyx(stdscr, row, col);

    WINDOW *peer_window = newwin(row, col / 2, 0, 0);
    // WINDOW *message_window = newwin(row, col / 2, 0, col / 2);

    int running = 1;
    while (running) {
        int ch = getch();

        switch (ch) {
        case KEY_ESC:
            running = 0;
            break;
        case KEY_UP:
            prev_peer(&selected_peer);
            break;
        case KEY_DOWN:
            next_peer(&selected_peer);
            break;
        case KEY_ENTER:
            // open chat/TCP connection
            break;
        }

        draw_peers(peer_window, selected_peer);
    }

    delwin(peer_window);
    endwin();
    return 0;
}
