#ifndef CONST_H
#define CONST_H

// שימוש במאקרואים פנימיים של GCC
typedef __builtin_va_list va_list;
#define va_start(v,l) __builtin_va_start(v,l)
#define va_arg(v,l)   __builtin_va_arg(v,l)
#define va_end(v)     __builtin_va_end(v)

// קבועי מערכת
#define true 1
#define false 0
#define null 0x00
#define bool int


#define ARROW_UP 0x48
#define ARROW_DOWN 0x50
#define ARROW_LEFT 0x4B
#define ARROW_RIGHT 0x4D
#define CONTROL_ON 0x1D
#define CONTROL_OFF 0x9D

// גדלי המסך
#define MAX_ROWS 200
#define SCREEN_ROWS 25
#define SCREEN_COLS 80

// הגדרות צבעים (VGA Color)
#define VIDEO_ADDRESS 0xB8000
#define RED_ON_BLACK 0x04
#define WHITE_ON_BLACK 0x0F
#define GREEN_ON_BLACK 0x0A
#define YELLOW_ON_BLACK 0x0E
#define BLUE_ON_BLACK 0x09
#define ORANGE_ON_BLACK 0x0C
#define SCAN_CODE_C 0x2E
#define SCAN_CODE_V 0x2F

// הצהרות לפונקציות
void clear_screen();
void printf(char* format, ...);
void print_float(float num , int precision);
void print_char(char c);
void print_int(int n);
void print_hex(unsigned int pointer);
int in_str(char* str , char c);
int len(char* str);
int countChar(char* str , char c);
int replace(char* str, char to_replace, char new_val);
void header_msg();
int strcmp(unsigned char* s1, char* s2);
void update_cursor();
int start_with(char* str, char* prefix);
int split(char* str, char c,char* parts[]);
char* strip(char* str);
int get_driver_name_size();
int min(int a, int b);
int max(int a, int b);
unsigned char port_byte_in(unsigned short port);
void port_byte_out(unsigned short port, unsigned char data);
void update_last_pos();
void move_line_right();
void move_line_left();
int get_safe_index();

extern int cur_color;
extern int current_view_row;
extern int cursor_pos;
extern int last_pos;
extern unsigned short terminal_buffer[MAX_ROWS * SCREEN_COLS];

#endif
