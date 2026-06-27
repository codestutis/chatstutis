#include "tui.h"
#include "boundedBuffer.h"
#include "chat.h"
#include "peer_discovery.h"
#include <ncurses.h>
#include <pthread.h>

#define KEY_ESC 27

static char msg_input_buf[MAX_MESSAGE_LENGTH];
static int msg_input_buf_end_idx;

static char msg_hist_buf[MAX_MSG_HIST_LEN];
static int msg_hist_buf_end_idx;
pthread_mutex_t hist_buf_mux = PTHREAD_MUTEX_INITIALIZER;

extern void *incoming_chat_buf;

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

void draw_peers(WINDOW *peer_w, peer_t selected_peer) {
    wclear(peer_w);
    pthread_mutex_lock(&peer_table_mux);
    for (int i = 0; i < MAX_PEERS; i++) {
        wmove(peer_w, i + 1, 1);
        if (strlen(peer_table[i].username) > 0) {
            if (strcmp(peer_table[i].username, selected_peer.username) == 0 &&
                strcmp(peer_table[i].addr, selected_peer.addr) == 0) {
                wattron(peer_w, A_BOLD | A_UNDERLINE);

                wprintw(peer_w, "%s, %s\n", peer_table[i].username,
                        peer_table[i].addr);

                wattroff(peer_w, A_BOLD | A_UNDERLINE);
            } else {
                wprintw(peer_w, "%s, %s\n", peer_table[i].username,
                        peer_table[i].addr);
            }
        }
    }
    pthread_mutex_unlock(&peer_table_mux);
    box(peer_w, 0, 0);
    wrefresh(peer_w);
}

void write_chat_history() {
    chat_packet_t *msg = tryGetBuffer(incoming_chat_buf);
    if (msg == NULL) {
        return;
    }

    pthread_mutex_lock(&hist_buf_mux);
    int written = snprintf(msg_hist_buf + msg_hist_buf_end_idx,
                           MAX_MSG_HIST_LEN - msg_hist_buf_end_idx, "%s: %s\n",
                           msg->from_usr, msg->message_data);
    if (written > 0) {
        msg_hist_buf_end_idx += written;
        if (msg_hist_buf_end_idx >= MAX_MSG_HIST_LEN) {
            msg_hist_buf_end_idx = MAX_MSG_HIST_LEN - 1;
        }
    }
    pthread_mutex_unlock(&hist_buf_mux);

    free(msg);
}

void draw_chat_hist(WINDOW *hist_w) {
    pthread_mutex_lock(&hist_buf_mux);
    wclear(hist_w);
    wmove(hist_w, 1, 1);
    wprintw(hist_w, "%.*s", msg_hist_buf_end_idx, msg_hist_buf);
    wrefresh(hist_w);
    pthread_mutex_unlock(&hist_buf_mux);
}

void append_buffer(char c) {
    if (msg_input_buf_end_idx < MAX_MESSAGE_LENGTH - 1) {
        msg_input_buf[msg_input_buf_end_idx++] = c;
    }
}

void flush_buffer() { msg_input_buf_end_idx = 0; }

void print_buffer(WINDOW *w) {
    wclear(w);
    wmove(w, 1, 1);
    wprintw(w, "> %.*s", msg_input_buf_end_idx, msg_input_buf);
}

char pop_buffer() {
    if (msg_input_buf_end_idx) {
        return msg_input_buf[--msg_input_buf_end_idx];
    }
    return -1;
}

int init_tui() {
    memset(msg_input_buf, 0, sizeof(msg_input_buf));
    msg_input_buf_end_idx = 0;
    memset(msg_hist_buf, 0, sizeof(msg_input_buf));
    msg_hist_buf_end_idx = 0;

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

    WINDOW *peer_w = newwin(row, col / 2, 0, 0);
    WINDOW *msg_hist_w = newwin((row / 5) * 4, col / 2, 0, col / 2);
    WINDOW *msg_input_w = newwin(row / 5, col / 2, (row / 5) * 4, col / 2);

    int running = 1;
    while (running) {
        int ch = getch();

        if (ch == KEY_ESC) {
            running = 0;
        } else if (ch == KEY_UP) {
            prev_peer(&selected_peer);
        } else if (ch == KEY_DOWN) {
            next_peer(&selected_peer);
        } else if (ch == '\n') {
            // send
            send_chat(&selected_peer, msg_input_buf, msg_input_buf_end_idx);
            flush_buffer();
        } else if (ch >= 32 && ch < 127) {
            append_buffer(ch);
        } else if (ch == 127) {
            // delete
            pop_buffer();
        }

        print_buffer(msg_input_w);
        draw_peers(peer_w, selected_peer);
        write_chat_history();
        draw_chat_hist(msg_hist_w);
        box(msg_hist_w, 0, 0);
        box(msg_input_w, 0, 0);
        wrefresh(msg_hist_w);
        wrefresh(msg_input_w);
    }

    delwin(peer_w);
    delwin(msg_hist_w);
    delwin(msg_input_w);
    endwin();
    return 0;
}
