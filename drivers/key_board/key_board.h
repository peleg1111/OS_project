#ifndef KEY_BOARD_H
#define KEY_BOARD_H


#include "stdio.h"
#include "terminal_command.h"
#include "key_board.h"

void key_board_handler();
void control_c_handler();
void control_v_handler();
void delete_txt_on_shift();
void clean_row_of_terminal();

#define COPY_TEXT_BUFFER_SIZE   SCREEN_COLS * 10

typedef struct {
    int index_of_shift;
    int on_shift;
    int on_control;
    int no_caps_lock;
    unsigned char *keyboard_map;
    unsigned char *shift_keyboard_map;
    char copy_text_buffer[COPY_TEXT_BUFFER_SIZE];
    
} KeyBoard;

extern KeyBoard key_board;

#endif
