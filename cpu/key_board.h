#ifndef KEY_BOARD_H
#define KEY_BOARD_H


#include "const.h"
#include "terminal_commend.h"
#include "key_board.h"

unsigned char port_byte_in(unsigned short port);
void port_byte_out(unsigned short port, unsigned char data);
void pic_remap();
void isr_handler(); // 0X60 קוראים ישירות מ

// אומר שהמערך קיים איפשהו
extern unsigned char keyboard_map[128];



#endif
