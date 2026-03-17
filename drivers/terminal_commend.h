#ifndef TERMINAL_COMMEND_H
#define TERMINAL_COMMEND_H

#include "const.h"

void print_to_commend_buffer(char c);
void handle_commend();
void clean_commend_buffer();
void terminal_init();
void change_color(char* parts[] , int size);
void help_commend();
extern unsigned char cmd_buffer[SCREEN_COLS];
extern int cmd_index;

#endif