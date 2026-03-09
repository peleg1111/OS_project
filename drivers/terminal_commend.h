#ifndef TERMINAL_COMMEND_H
#define TERMINAL_COMMEND_H

#include "const.h"

void print_to_commend_buffer(char c);
void handle_commend();
void clean_commend_buffer();
extern unsigned char cmd_buffer[SCREEN_COLS];
extern int cmd_index;

#endif