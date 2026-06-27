#ifndef TUI_H
#define TUI_H

#include "chat.h"
#include <ncurses.h>

#define MAX_MSG_HIST_LEN 4096

void append_buffer(char c);
void flush_buffer();

int init_tui();

void print_message();
#endif
