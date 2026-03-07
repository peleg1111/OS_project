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

// גדלי המסך
#define MAX_ROWS 100
#define SCREEN_ROWS 25
#define SCREEN_COLS 80

// הגדרות צבעים (VGA Color)
#define VIDEO_ADDRESS 0xB8000
#define RED_ON_BLACK 0x04
#define WHITE_ON_BLACK 0x0F
#define GREEN_ON_BLACK 0x0A
#define YELLOW_ON_BLACK 0x0E

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


#endif
