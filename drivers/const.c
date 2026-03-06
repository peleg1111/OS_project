#include "const.h"

static int cursor_pos = 0; // משתנה שזוכר איפה עצרנו
    volatile unsigned short* video = (unsigned short*) VIDEO_ADDRESS;
static int cur_color = WHITE_ON_BLACK;

// חוצץ ששומר את כל הטקסט (תו + צבע)
unsigned short terminal_buffer[MAX_ROWS * SCREEN_COLS];
int current_view_row = 0; // השורה שאותה אנחנו רואים כרגע בראש המסך

void update_screen() {
    // הגדרה כ-short מאפשרת לנו להעתיק תו + צבע בפקודה אחת
    for (int i = 0; i < SCREEN_ROWS; i++) {
        for (int j = 0; j < SCREEN_COLS; j++) {
            int buffer_index = (current_view_row + i) * SCREEN_COLS + j;
            video[i * SCREEN_COLS + j] = terminal_buffer[buffer_index];
        }
    }
}

void print_char_to_buffer(char c, int row, int col) {
    //  מקדם את הצבע לפני התו וה-| מחבר בניהם לביט אחד 
    terminal_buffer[row * SCREEN_COLS + col] = (c | (cur_color << 8));
}


void set_color(int new_color){
    cur_color = new_color;
}


// פונקציה לניקוי המסך
void clear_screen() {
    for (int i = 0; i < SCREEN_COLS * MAX_ROWS; i += 2) {
        terminal_buffer[i] = ' ' | cur_color<<8;
    }
    cursor_pos = 0;
    current_view_row = 0;
    update_screen();

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
    // מניעת חריגה מה-Buffer הכללי (למשל 1000 שורות)
    if (cursor_pos / 2 >= MAX_ROWS * SCREEN_COLS) {
        return; 
    }

    if (c == '\n') {
        cursor_pos = ((cursor_pos / (SCREEN_COLS * 2)) + 1) * (SCREEN_COLS * 2);
    } else {
        terminal_buffer[cursor_pos / 2] = (unsigned short)c | (unsigned short)(cur_color << 8);
        cursor_pos += 2;
    }
    
    // בדיקה: האם הסמן עבר את גבול המסך הנוכחי (25 שורות מהתחלת התצוגה)?
    int current_cursor_row = cursor_pos / (SCREEN_COLS * 2);
    if (current_cursor_row >= current_view_row + SCREEN_ROWS) {
        current_view_row = current_cursor_row - SCREEN_ROWS + 1;
    }

    update_screen();
}

void print_int(int num) {

    //  בדיקת גלילה בתוך ה-Buffer (אם הגענו לסוף הזיכרון שהקצינו ב-RAM)
    if (cursor_pos / 2 >= MAX_ROWS * SCREEN_COLS) {
        // כאן נצטרך לוגיקת גלילה ל-Buffer עצמו אם נרצה לשמור היסטוריה אינסופית
        // כרגע פשוט נמנע חריגה
        return; 
    }

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

    //  בדיקת גלילה בתוך ה-Buffer (אם הגענו לסוף הזיכרון שהקצינו ב-RAM)
    if (cursor_pos / 2 >= MAX_ROWS * SCREEN_COLS) {
        // כאן נצטרך לוגיקת גלילה ל-Buffer עצמו אם נרצה לשמור היסטוריה אינסופית
        // כרגע פשוט נמנע חריגה
        return; 
    }
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


