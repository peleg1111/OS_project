#include "stdio.h"
#include "key_board.h"
#include "timer.h"
#include "thread.h"

void clean_command_history();

 Print_info print_info = {
    .cursor_pos = 0, 
    .video = (unsigned short*) (VIDEO_ADDRESS),
    .cur_color = WHITE_ON_BLACK,
    .driver_name = "enter command-->",
    .last_pos = 0,
    .current_view_row = 0,
    .last_header_pos = 0,
    .terminal_buffer = {0},
    .buffer_row_offset = 0,
    .text_offset = 0
};

const char* error_messages[]  = {
    "Success",
    "Buffer overflow",
    "Invalid command",
    "Unknown error"
};

void update_screen()
{

    for (int i = print_info.text_offset; i < SCREEN_ROWS; i++)
    {
        for (int j = 0; j < SCREEN_COLS; j++)
        {

            int buffer_row = i + print_info.text_offset + print_info.current_view_row;
            buffer_row -= print_info.buffer_row_offset;

            if (buffer_row < MAX_ROWS - 1 && buffer_row >= 0)
            {
                int buffer_index = buffer_row * SCREEN_COLS + j;
                if (buffer_index >= 0 && buffer_index < MAX_ROWS * SCREEN_COLS)
                {

                    int start_sel = min(print_info.cursor_pos, key_board.index_of_shift) / 2;
                    int end_sel = max(print_info.cursor_pos, key_board.index_of_shift) / 2;

                    if (key_board.index_of_shift != -1 && buffer_index >= start_sel && buffer_index < end_sel)
                    {
                        print_info.video[i * SCREEN_COLS + j] = (print_info.terminal_buffer[buffer_index] & 0x00FF) | (ORANGE_ON_BLACK << 8);
                    }
                    else
                    {
                        print_info.video[i * SCREEN_COLS + j] = print_info.terminal_buffer[buffer_index];
                    }
                }
            }
            else
            {
                clear_screen();
                header_msg();
            }
        }
    }
    update_cursor();

}

// משנה צבע
void set_color(int new_color){
    print_info.cur_color = new_color;
}

// מדפיס את הודעת הראש שיש בכל שורה
void header_msg(){

    printf("\n\n%s", print_info.driver_name);
    update_cursor();
    update_screen();
    print_info.last_header_pos = print_info.cursor_pos;
}

int get_driver_name_size(){
    return len(print_info.driver_name);
}

// פונקציה לניקוי המסך
void clear_screen() {
    for (int i = 0; i < SCREEN_COLS * MAX_ROWS; i ++) {
        if (i >= 0 && i < MAX_ROWS * SCREEN_COLS) {
            if (i >= 0 && i < MAX_ROWS * SCREEN_COLS) {
                if (i >= 0 && i < MAX_ROWS * SCREEN_COLS) {
                    print_info.terminal_buffer[i] = ' ' | WHITE_ON_BLACK<<8;
                }
            }
        }
    }
    print_info.cursor_pos = 0;
    print_info.last_pos = 0;
    print_info.current_view_row = 0;
    print_info.last_header_pos = 0;
    print_info.buffer_row_offset = 0;

    update_screen();
    update_cursor();
    clean_command_history();

}

void printf (char* message, ...) {
    
    print_info.buffer_row_offset = 0;
    va_list args;
    va_start(args,message);
    for (int i = 0; message[i] != null; i++) 
    {
        if(message[i] == '%')
        {
            i++;
            if(message[i] == 's'){
                // הקוד המתוקן והנכון ארכיטקטונית
                char* c = va_arg(args, char*);
                for (int index = 0 ; c[index] != null && print_info.last_pos < MAX_ROWS * SCREEN_COLS; index++){
                    print_char(c[index]);   
                }
            }

            else if(message[i] == 'c'){
                char c = (char)va_arg(args, int);
                print_char(c);
            }  

            else if (message[i] == 'd'){
                int num = va_arg(args,int);
                print_int(num);
            }

            else if (message[i] == 'p' || message[i] == 'x'){
                unsigned int ptr = va_arg(args, unsigned int);
                print_hex(ptr);
            }

            else if(message[i]>='0' && message[i]<='9'){
                int f_num = message[i]-'0';
                i++;
                if (message[i] == 'f'){
                float num = (float)va_arg(args,double);
                print_float(num,f_num);
                }
            }
        }

        else
        {
            print_char(message[i]);
        }
    }
    update_screen();
    update_cursor();
    
}


void print_char(char c) {

    if (c == '\n') {
        print_info.cursor_pos = ((print_info.last_pos / (SCREEN_COLS * 2)) + 1) * (SCREEN_COLS * 2);
        print_info.last_pos = print_info.cursor_pos;
    }

    else if (c == '\b') { // מחיקה (Backspace)
        if (print_info.cursor_pos > 0 && (print_info.cursor_pos / 2) > get_safe_index()) {

            print_info.cursor_pos -= 2;
            print_info.last_pos -= 2;
            int idx = print_info.cursor_pos / 2;
            if (idx >= 0 && idx < MAX_ROWS * SCREEN_COLS && print_info.cursor_pos/2 >= get_safe_index()) {
                if(print_info.cursor_pos < print_info.last_pos){
                    move_line_left();
                }
                print_info.terminal_buffer[print_info.last_pos/2] = ' ' | (unsigned short)(WHITE_ON_BLACK << 8);
            }
        }
    }

    else if (c == '\t'){
        for(int i = 0; i<4;i++){
        int idx = print_info.cursor_pos / 2;
        if (idx >= 0 && idx < MAX_ROWS * SCREEN_COLS) {
            print_info.terminal_buffer[idx] = ' ' | (unsigned short)(print_info.cur_color << 8);
        }
        print_info.cursor_pos += 2;
        print_info.last_pos += 2;
        }
    }
    else {
        int idx = print_info.cursor_pos / 2;
        if (idx >= 0 && idx < MAX_ROWS * SCREEN_COLS) {
            if(print_info.cursor_pos < print_info.last_pos){
                move_line_right();
            }
            print_info.terminal_buffer[idx] = (unsigned short)c | (unsigned short)(print_info.cur_color << 8);
        }
        print_info.cursor_pos += 2;
        print_info.last_pos += 2;
    }

    print_info.current_view_row = max(0,(print_info.last_pos / (SCREEN_COLS * 2) - SCREEN_ROWS));

}



void print_int(int num) {
    int old_color = print_info.cur_color;
    set_color(YELLOW_ON_BLACK);

    if (num == 0) {
        print_char('0');
    } else {
        if (num < 0) {
            print_char('-');
            num = -num;
        }

        char digits_str[12];
        int i = 0;
        while (num > 0) {
            digits_str[i++] = (num % 10) + '0';
            num /= 10;
        }

        int idx = print_info.cursor_pos / 2;
        // הדפסה בסדר הפוך כיוון שרשמתי את הערכים בסדר הפוך
        for (int index = i - 1; idx < MAX_ROWS * SCREEN_COLS && index >= 0; index--)
        {
            print_char(digits_str[index]);
        }
    }
    set_color(old_color);
}



void print_hex(unsigned int pointer){

    int old_color = print_info.cur_color;
    set_color(GREEN_ON_BLACK);

    char* hex = "0123456789ABCDEF";
    char buffer[11];
    buffer[0] = '0';
    buffer[1] = 'X';
    buffer[10] = null;
    int idx = print_info.cursor_pos / 2;

    for (int i = 9; i >= 2 && idx < MAX_ROWS * SCREEN_COLS; i--)
    {
        buffer[i] = hex[pointer & 0xF];
        pointer >>=4; // מקדם את המצביעה 4 ביטים קדימה ושומר את הקודמים להדפסה
    }

    for (int i = 0; buffer[i] != null && idx < MAX_ROWS * SCREEN_COLS; i++)
    {
        print_char(buffer[i]);
    }
    set_color(old_color);
}


void print_float(float num, int precision) {

    int old_color = print_info.cur_color;
    set_color(YELLOW_ON_BLACK);
    // טיפול במספרים שליליים
    if (num < 0) {
        print_char('-');
        num *= -1;
    }

    int int_num = (int)num;
    print_int(int_num);

    print_char('.');

    float f_num = num - (float)int_num; 
    
    for (int i = 0; i < precision; i++) {
        f_num *= 10;
        int digit = (int)f_num;
        print_char(digit + '0');
        f_num -= (float)digit;
    }
    set_color(old_color);
}

// מחזיר את האורך של מחרוזת
int len(char* str){
    int i = 0;
    while(str[i]!=null){
        i++;
    }
    return i;
}


// מחליף את כל האותיות הרצויות באותיות אחרות ומחזיר את הכמות שהפעולה החליפה
int replace(char* str, char to_replace, char new_val){
    int i = 0;
    int num = 0;
    while(str[i] != null){
        if(str[i] == to_replace){
            str[i] = new_val;
            num++;
        }
        i++;
    }
    return num;
}
 

//  השווה בין מחרוזות
int strcmp(unsigned char* s1, char* s2) {
    int i = 0;
    while (s1[i] != '\0' && s2[i] != '\0') {
        if (s1[i] != s2[i]) return 0;
        i++;
    }
    return (s1[i] == s2[i]) ? 1 : 0;
}


// קבלת קלט מהמחשב
unsigned char port_byte_in(unsigned short port) {
    unsigned char result;
    __asm__ __volatile__ ("inb %1, %0" : "=a" (result) : "nd" (port));
    return result;
}

// שולח מידע לפורט מסוים במחשב
void port_byte_out(unsigned short port, unsigned char data) {
    __asm__ __volatile__ ("outb %0, %1" : : "a" (data), "nd" (port));
}

// מעדכן את המיקום של הסמן
void update_cursor(){

    int pos = print_info.cursor_pos/2 - SCREEN_COLS * print_info.text_offset;
    while(pos >= SCREEN_COLS * SCREEN_ROWS ){
        pos -= SCREEN_COLS;
    }


    port_byte_out(0x3D4 , 0x0E);
    port_byte_out(0x3D5, ( pos >> 8) & 0xFF);
    port_byte_out(0x3D4 , 0x0F);
    port_byte_out(0x3D5, pos & 0xFF);
}


// בודק האם המחרוזת מתחילה ברצף מסוים של תווים
int starts_with(char* str, char* c) {
    int i = 0;
    while (c[i] != '\0') {
        if (str[i] != c[i]) return 0;
        i++;
    }
    return 1;   
}

// את החלקים ומחזיר את כמות החלקים (parts) שם ב 
int split(char* str, char c,char* parts[]){
    int l = len(str);
    int index = 1;
    parts[0] = str; 
    for(int i = 0; i< l; i++){
        if(str[i] == c){
            str[i] = null;
            if(str[i+1] != c){
                parts[index] = &str[i+1];
                index++;
            }
        }
    }
    return index;
}


// מחליף לאותיות קטנות
void lower(char* str){
    for(int i = 0; str[i] != null; i++){
        if(str[i]>='A' && str[i]<='Z'){
            str[i] += (int)('a'-'A'); 
        }
    }
}

// מוריד רווחים מהסוף וההתחלה
char* strip(char* str){
    while(str[0] == ' '){
        str += 1;
    }
    int i = 0;
    while(str[i]!= null){
        i++;
    }
    i--;
    while(str[i] == ' '){
        str[i] = null;
        i--;
    }
    return str;
}


int min(int a, int b){
    return (a < b) ? a : b;
}


int max(int a, int b){
    return (a > b) ? a : b;
}


void move_line_right(){
    for (int i = print_info.last_pos; i > print_info.cursor_pos; i--){
        print_info.terminal_buffer[i/2 +1] = print_info.terminal_buffer[i/2];
    }
}


void move_line_left(){
    for (int i = print_info.cursor_pos; i < print_info.last_pos; i++){
        print_info.terminal_buffer[i/2] = print_info.terminal_buffer[i/2 +1];
    }
    
}


// מחזיר את המיקום שלמשתמש מותר לכתוב בו(אסור לו לכתוב במיקומים שלפני)
int get_safe_index(){
    return print_info.last_header_pos/2;
}

int abs(int x , int y){
    x -= y;
    if(x < 0){
        return -x;
    }
    return x;
}