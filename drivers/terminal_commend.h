#ifndef TERMINAL_COMMEND_H
#define TERMINAL_COMMEND_H

#include "const.h"

void print_to_commend_buffer(char c);
void handle_commend();
void clean_commend_buffer();
void terminal_init();
void change_color(char* parts[] , int size);
void help_commend();
void clean_commend_history();
void insert_to_history(char* cmd);
void display_history();
void get_from_history(int index);
void refresh_cmd_buffer();


extern int display_history_index;
extern int size_of_history;
extern unsigned char cmd_buffer[SCREEN_COLS];
extern int cmd_index;
extern char commends_history[SCREEN_ROWS * MAX_ROWS][SCREEN_COLS * 5];
#endif