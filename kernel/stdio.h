#ifndef CONST_H
#define CONST_H

#include "thread.h"

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
#define ON_SHIFT_1 0x2A
#define ON_SHIFT_2 0x36
#define OFF_SHIFT_1 0xAA
#define OFF_SHIFT_2 0xB6
#define CAP_LOCK_ON 0x3A
#define A_KEY_UP 0x1E
#define C_KEY_UP 0x2E
#define V_key_UP 0x2F

// גדלי המסך
#define MAX_ROWS 200
#define SCREEN_ROWS 25
#define SCREEN_COLS 80

// הגדרות צבעים (VGA Color)
#define VIDEO_ADDRESS 0xB8000
#define BLACK_ON_BLACK 0x00         // שחור
#define BLUE_ON_BLACK 0x01          // כחול כהה
#define GREEN_ON_BLACK 0x02         // ירוק כהה
#define CYAN_ON_BLACK 0x03          // ציאן (טורקיז) כהה
#define RED_ON_BLACK 0x04           // אדום כהה
#define MAGENTA_ON_BLACK 0x05       // סגול (מג'נטה)
#define BROWN_ON_BLACK 0x06         // חום (צהוב כהה)
#define LIGHT_GRAY_ON_BLACK 0x07    // אפור בהיר
#define DARK_GRAY_ON_BLACK 0x08     // אפור כהה
#define LIGHT_BLUE_ON_BLACK 0x09    // כחול בהיר
#define LIGHT_GREEN_ON_BLACK 0x0A   // ירוק בהיר
#define LIGHT_CYAN_ON_BLACK 0x0B    // ציאן (טורקיז) בהיר
#define ORANGE_ON_BLACK 0x0C        // אדום בהיר / כתום
#define LIGHT_MAGENTA_ON_BLACK 0x0D // סגול בהיר / ורוד
#define YELLOW_ON_BLACK 0x0E        // צהוב
#define WHITE_ON_BLACK 0x0F         // לבן בוהק

// הצהרות לפונקציות

// פעולות להדפסה למסך
void printf(char* format, ...);
void print_float(float num , int precision);
void print_char(char c);
void print_int(int n);
void print_unsigned_int(unsigned int n);
void print_hex(unsigned int pointer);

//    פעולות עזר למחרוזות
int in_str(char* str , char c);
int len(char* str);
int countChar(char* str , char c);
int replace(char* str, char to_replace, char new_val);
int strcmp(unsigned char* s1, char* s2);
int start_with(char* str, char* prefix);
int split(char* str, char c,char* parts[]);
char* strip(char* str);

void header_msg();
int get_driver_name_size();


// פעולות עזר מתמטיקה
int min(int a, int b);
int max(int a, int b);

// פעולות עזר עבודה עם החומרה + עדכוני מסך
unsigned char port_byte_in(unsigned short port);
void port_byte_out(unsigned short port, unsigned char data);
// עדכוני מסך
void clear_screen();
void move_line_right();
void move_line_left();
void update_cursor();
int get_safe_index();
void set_color(int new_color);
int abs(int x , int y);

typedef struct {
    int cursor_pos; 
    volatile unsigned short* video;
    int cur_color;
    char driver_name[20];
    int last_pos;
    int current_view_row; // השורה שאותה אנחנו רואים כרגע בראש המסך
    int last_header_pos;
    unsigned short terminal_buffer[MAX_ROWS * SCREEN_COLS];// חוצץ ששומר את כל הטקסט (תו + צבע)
    unsigned short buffer_row_offset;
    int text_offset;

} Print_info;

extern Print_info print_info;


typedef enum {
    SUCCESS = 0,
    BUFFER_OVERFLOW = 1,
    INVALID_COMMAND = 2,
    UNKNOWN_ERROR = 3

} Error_code;


extern const char* error_messages[];

#define yield __asm__ __volatile__("int $0x81")
#define uint32 unsigned int

#endif
