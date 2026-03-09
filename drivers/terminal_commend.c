#include "const.h"
#include "terminal_commend.h"

unsigned char cmd_buffer[SCREEN_COLS];
int cmd_index = 0;


void clean_commend_buffer() {
    for(int i = 0; i < SCREEN_COLS; i++) {
        cmd_buffer[i] = null;
    }
    cmd_index = 0;
}

void handle_commend() {
    if(strcmp(cmd_buffer,"clean") == 1){
        clear_screen();
    }
    clean_commend_buffer();
}


void print_to_commend_buffer(char c) {
    if (c == '\n') {
        if (c == '\n') {
            cmd_buffer[cmd_index] = '\0';
            return; 
        }
    } 

    else if( c == '\b'){
        cmd_buffer[cmd_index] = null;
        cmd_index--;
        if(cmd_index < 0){
            cmd_index = 0;
        }
    }
    else{
        if (cmd_index < SCREEN_COLS - 1) {
            cmd_buffer[cmd_index] = c;
            cmd_index++;
        }
    }
}
