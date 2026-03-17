#ifndef KEY_BOARD_H
#define KEY_BOARD_H


#include "const.h"
#include "terminal_commend.h"
#include "key_board.h"


void pic_remap();
void isr_handler();
void wait() ;
void control_c_handler();
void control_v_handler();
void delete_txt_on_shift();
void clean_row_of_terminal();

extern unsigned char keyboard_map[128];
extern int index_of_shift;
extern int on_shift;

#endif
