#ifndef TERMINAL_COMMAND_H
#define TERMINAL_COMMAND_H
#include "stdio.h"

void print_to_command_buffer(char c);
void handle_command();
void clean_command_buffer();
void change_color(char* parts[] , int size);
void clean_command_history();
void insert_to_history(char* cmd);
void display_history();
void get_from_history(int index);
void refresh_cmd_buffer();
void terminal_init();

#define COMMANDS_NUMBER 4

#define COMMAND_BUFFER_SIZE SCREEN_COLS * 5

    typedef struct {

    int size_of_history;
    
    int display_history_index;
    int cmd_index;
    
    void (*handle_command)();
    void (*clean_command_buffer)();
    void (*change_color)(char* parts[] , int size);
    void (*clean_command_history)();
    void (*insert_to_history)(char* cmd);
    void (*display_history)();
    
    char commands_history[SCREEN_ROWS * MAX_ROWS][COMMAND_BUFFER_SIZE];
    unsigned char cmd_buffer[COMMAND_BUFFER_SIZE];

} Terminal_command;

extern Terminal_command terminal_command;


#endif