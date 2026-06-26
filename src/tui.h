#ifndef TUI_H
#define TUI_H

#include "chat.h"
#include <ncurses.h>

#define MAX_MSG_HIST_LEN 4096

static char msg_input_buf[MAX_MESSAGE_LENGTH];
static int msg_input_buf_end_idx;

static char msg_hist_buf[MAX_MSG_HIST_LEN];

void append_buffer(char c);
void flush_buffer();

int init_tui();

void print_message();
#endif