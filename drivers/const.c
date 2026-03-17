#include "const.h"
#include "key_board.h"
int cursor_pos = 0; // משתנה שזוכר איפה עצרנו
volatile unsigned short* video = (unsigned short*) (VIDEO_ADDRESS);
int cur_color = WHITE_ON_BLACK;
char driver_name[20] = "enter commend-->";
int last_pos = 0;
// חוצץ ששומר את כל הטקסט (תו + צבע)
unsigned short terminal_buffer[MAX_ROWS * SCREEN_COLS];
int current_view_row = 0; // השורה שאותה אנחנו רואים כרגע בראש המסך

void clean_commend_history();

void update_screen() {
    for (int i = 0; i < SCREEN_ROWS; i++) {
        for (int j = 0; j < SCREEN_COLS; j++) {
            int buffer_row = current_view_row + i;
            // מניעת חריגה
            if (buffer_row < MAX_ROWS) {
                int buffer_index = buffer_row * SCREEN_COLS + j;
                if (buffer_index >= 0 && buffer_index < MAX_ROWS * SCREEN_COLS) {
                    
                    int buffer_index = buffer_row * SCREEN_COLS + j;
                    int start_sel = min(cursor_pos, index_of_shift) / 2;
                    int end_sel = max(cursor_pos, index_of_shift) / 2;

                    if (index_of_shift != -1 && buffer_index >= start_sel && buffer_index < end_sel) {
                        video[i * SCREEN_COLS + j] = (terminal_buffer[buffer_index] & 0x00FF) | (ORANGE_ON_BLACK << 8);
                    } 
                    else {
                        video[i * SCREEN_COLS + j] = terminal_buffer[buffer_index];
                    }

                }
            }
            else {
                clear_screen();
            }
        }
    }
    update_cursor();
}

void print_char_to_buffer(char c, int row, int col) {
    //  מקדם את הצבע לפני התו וה-| מחבר בניהם לביט אחד 
    int idx = row * SCREEN_COLS + col;
    if (idx >= 0 && idx < MAX_ROWS * SCREEN_COLS) {
        terminal_buffer[idx] = (c | (cur_color << 8));
    }
}


void set_color(int new_color){
    cur_color = new_color;
}
void header_msg(){
    printf("\n\n%s",driver_name);
}
int get_driver_name_size(){
    return len(driver_name);
}

// פונקציה לניקוי המסך
void clear_screen() {
    for (int i = 0; i < SCREEN_COLS * MAX_ROWS; i ++) {
        if (i >= 0 && i < MAX_ROWS * SCREEN_COLS) {
            if (i >= 0 && i < MAX_ROWS * SCREEN_COLS) {
                if (i >= 0 && i < MAX_ROWS * SCREEN_COLS) {
                    terminal_buffer[i] = ' ' | cur_color<<8;
                }
            }
        }
    }
    cursor_pos = 0;
    last_pos = 0;
    current_view_row = 0;
    update_screen();
    clean_commend_history();

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
                for (int index = 0 ; c[index] != null; index++){

                    print_char(c[index]);   
                }
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

            else if(message[i]>='0' && message[i]<='9'){
                int f_num = message[i]-'0';
                i++;
                if (message[i] == 'f'){
                float num = va_arg(args,double);
                print_float(num,f_num);
                }
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
        cursor_pos = ((last_pos / (SCREEN_COLS * 2)) + 1) * (SCREEN_COLS * 2);
        last_pos = cursor_pos;
    }

    else if (c == '\b') { // מחיקה (Backspace)
        if (cursor_pos > 0 && (cursor_pos / 2) > get_safe_index()) {
            cursor_pos -= 2;
            last_pos -= 2;
            int idx = cursor_pos / 2;
            if (idx >= 0 && idx < MAX_ROWS * SCREEN_COLS && cursor_pos/2 >= get_safe_index()) {
                if(cursor_pos < last_pos){
                    move_line_left();
                }
                terminal_buffer[last_pos/2] = ' ' | (unsigned short)(cur_color << 8);
            }
        }
    }

    else if (c == '\t'){
        for(int i = 0; i<4;i++){
        int idx = cursor_pos / 2;
        if (idx >= 0 && idx < MAX_ROWS * SCREEN_COLS) {
            terminal_buffer[idx] = ' ' | (unsigned short)(cur_color << 8);
        }
        cursor_pos += 2;
        last_pos += 2;
        }
    }
    else {
        int idx = cursor_pos / 2;
        if (idx >= 0 && idx < MAX_ROWS * SCREEN_COLS) {
            if(cursor_pos < last_pos){
                move_line_right();
            }
            terminal_buffer[idx] = (unsigned short)c | (unsigned short)(cur_color << 8);
        }
        cursor_pos += 2;
        last_pos += 2;
    }

    // האם הסמן עבר את גבול המסך הנוכחי 
    int current_cursor_row = cursor_pos / (SCREEN_COLS * 2);
    if (current_cursor_row >= current_view_row + SCREEN_ROWS) {
        current_view_row = current_cursor_row - SCREEN_ROWS + 1;
    }

    update_screen();
}

void print_int(int num) {
    int old_color = cur_color;
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

        // הדפסה בסדר הפוך כיוון שרשמתי את הערכים בסדר הפוך
        for (int index = i - 1; index >= 0; index--) {
            print_char(digits_str[index]);
        }
    }
    set_color(old_color);
}


void print_hex(unsigned int pointer){

    int old_color = cur_color;
    set_color(BLUE_ON_BLACK);

    char* hex = "0123456789ABCDEF";
    char buffer[11];
    buffer[0] = '0';
    buffer[1] = 'X';
    buffer[10] = null;
    for(int i = 9; i>=2;i-- ){
        buffer[i] = hex[pointer & 0xF];
        pointer >>=4; // מקדם את המצביעה 4 ביטים קדימה ושומר את הקודמים להדפסה
    }

    for (int i = 0; buffer[i] != null; i++) {
        print_char(buffer[i]);
    }
    set_color(old_color);
}


void print_float(float num, int precision) {

    int old_color = cur_color;
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

void port_byte_out(unsigned short port, unsigned char data) {
    __asm__ __volatile__ ("outb %0, %1" : : "a" (data), "nd" (port));
}

void update_cursor(){
    int pos = cursor_pos;
    int size = SCREEN_COLS * SCREEN_ROWS * 2;
    int down = SCREEN_COLS * 2;
    while(pos >= (size)){
        pos -= down;
    }
    pos = pos/2;

    port_byte_out(0x3D4 , 0x0E);
    port_byte_out(0x3D5, ( pos >> 8) & 0xFF);
    port_byte_out(0x3D4 , 0x0F);
    port_byte_out(0x3D5, pos & 0xFF);
}


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

void lower(char* str){
    for(int i = 0; str[i] != null; i++){
        if(str[i]>='A' && str[i]<='Z'){
            str[i] += (int)('a'-'A'); 
        }
    }
}

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
void update_last_pos(){
    while(cursor_pos > last_pos){
        last_pos += 2;
    }
}
void move_line_right(){
    for (int i = last_pos; i > cursor_pos; i--){
        terminal_buffer[i/2 +1] = terminal_buffer[i/2];
    }
}
void move_line_left(){
    for (int i = cursor_pos; i < last_pos; i++){
        terminal_buffer[i/2] = terminal_buffer[i/2 +1];
    }
    
}
int get_safe_index(){
    return((cursor_pos / 2)/SCREEN_COLS)*SCREEN_COLS + get_driver_name_size();
}