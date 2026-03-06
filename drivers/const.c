#include "help_func.h"

static int cursor_pos = 0; // משתנה שזוכר איפה עצרנו
volatile char* vidmem = (char*) VIDEO_ADDRESS;
static int cur_color = WHITE_ON_BLACK;

void set_color(int new_color){
    cur_color = new_color;
}


// פונקציה לניקוי המסך
void clear_screen() {
    for (int i = 0; i < 80 * 25 * 2; i += 2) {
        vidmem[i] = ' ';
        vidmem[i+1] = cur_color;
    }
}

// פונקציה להדפסת מחרוזת
void printf (char* message, ...) {
    va_list args;
    va_start(args,message);

    for (int i = 0; message[i] != null; i++) 
    {
        if(message[i] == '%')
        {
            i++;

            if(message[i] == 's'){
                char* c = (char*)va_arg(args,int);
                int old_color = cur_color;
                set_color(GREEN_ON_BLACK);
                for (int index = 0 ; c[index] != null; index++){

                    print_char(c[index]);   
                }
                set_color(old_color);
            }

            else if(message[i] == 'c'){
                char c = (char)va_arg(args,int);
                int old_color = cur_color;
                set_color(YELLOW_ON_BLACK);
                print_char(c);
                set_color(old_color);
            }  

            else if (message[i] == 'd'){
                int num = va_arg(args,int);
                print_int(num);
            }

            else if (message[i] == 'p'){
                unsigned int ptr = va_arg(args, unsigned int);
                print_hex(ptr);
            }

        }
        else
        {
            print_char(message[i]);
        }
    }
}


void print_char(char c) {
    if (c == '\n') {
        cursor_pos = ((cursor_pos / 160) + 1) * 160;
    } else {
        vidmem[cursor_pos] = c;
        vidmem[cursor_pos + 1] = cur_color;
        cursor_pos += 2;
    }
}

void print_int(int num) {
    int old_color = cur_color; // שמירת הצבע הנוכחי
    set_color(RED_ON_BLACK);

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

        // הדפסה בסדר הפוך כיוון שרשמתי את הערכים בסדר הפוך
        for (int index = i - 1; index >= 0; index--) {
            print_char(digits_str[index]);
        }
    }
    set_color(old_color);
}


void print_hex(unsigned int pointer){
    char* hex = "0123456789ABCDEF";
    char buffer[11];
    buffer[0] = '0';
    buffer[1] = 'X';
    buffer[10] = null;
    for(int i = 9; i>=2;i-- ){
        buffer[i] = hex[pointer & 0xF];
        pointer >>=4; // מקדם את המצביעה 4 ביטים קדימה ושומר את הקודמים להדפסה
    }
    int old_color = cur_color;
    set_color(GREEN_ON_BLACK);
    for (int i = 0; buffer[i] != null; i++) {
        print_char(buffer[i]);
    }
    set_color(old_color);
}