#include "const.h"
#include "terminal_commend.h"

void update_screen();
unsigned char port_byte_in(unsigned short port);
void port_byte_out(unsigned short port, unsigned char data);
void control_c_handler();
void control_x_handler();
void control_v_handler();
void refresh_cmd_buffer();
void delete_txt_on_shift();
void handle_normal_press(char scan_code);
void clean_row_of_terminal();


char copy_text_buffer[SCREEN_COLS * 5];

// לתווים בעת לחיצה על המקלדת hex תרגום מ
// 0 --> מקש שאיןן לו טיפול
unsigned char keyboard_map[128] = {
    0,  27, '1', '2', '3', '4', '5', '6', '7', '8',
    '9', '0', '-', '=', '\b',	/* Backspace */
    '\t',			/* Tab */
    'q', 'w', 'e', 'r',
    't', 'y', 'u', 'i', 'o', 'p', '[', ']', '\n',
    0,			/* Control */
    'a', 's', 'd', 'f', 'g', 'h', 'j', 'k', 'l', ';',
    '\'', '`',   0,		/* Left shift */
    '\\', 'z', 'x', 'c', 'v', 'b', 'n',
    'm', ',', '.', '/',   0, /* Right shift */
    '*',
    0,	/* Alt */
    ' '	/* Space bar */
};

unsigned char shift_keyboard_map[128] = {
    0, 27, '!', '@', '#', '$', '%', '^', '&', '*', '(', ')', '_', '+', '\b', 
    '\t', 'Q', 'W', 'E', 'R', 'T', 'Y', 'U', 'I', 'O', 'P', '{', '}', '\n', 
    0, 'A', 'S', 'D', 'F', 'G', 'H', 'J', 'K', 'L', ':', '\"', '~', 
    0, '|', 'Z', 'X', 'C', 'V', 'B', 'N', 'M', '<', '>', '?', 0, 0, 0, ' '
};

int on_shift = 0;
int no_caps_lock = 1;
int on_control = 0;
int index_of_shift = -1;

// מחכה קצת כדי למנוע בעיות של קלט מהיר מדי
void wait() {
    for (int i = 0; i < 500000; i++) {
        unsigned char scan_code = port_byte_in(0x60);
        port_byte_out(0x20, 0x20); // EOI --> משחרר את ההאזנה על המקלדת כדי להמשיך לקלוט מקשים
    }
}

void pic_remap(){
    
    port_byte_out(0x20, 0x11);
    port_byte_out(0x21, 0x20);
    port_byte_out(0x21,0x04);
    port_byte_out(0x21,0x01);

    port_byte_out(0xA0, 0x11);
    port_byte_out(0xA1, 0x28);  
    port_byte_out(0xA1,0x02);
    port_byte_out(0xA1,0x01);

    port_byte_out(0x21, 0xFD); 
    port_byte_out(0xA1, 0xFF); // Slav חוסם את כל השאר ב 
    
}

void isr_handler() {
    unsigned char scan_code = port_byte_in(0x60);
    port_byte_out(0x20, 0x20); // EOI --> משחרר את ההאזנה על המקלדת כדי להמשיך לקלוט מקשים
    
    //shift טיפול בעזיבה של המקש 
    if(scan_code == 0xB6 || scan_code == 0xAA) {
        on_shift = 0;
    }
    //shift טיפול בלחיצה על מקש
    else if (scan_code == 0x36 || scan_code == 0x2A) {
        on_shift = 1;
        index_of_shift = cursor_pos;
    }
    //caps lock טיפול בלחיצה על מקש
    else if(scan_code == 0x3A){
        no_caps_lock = !no_caps_lock;
    }

    else if( scan_code == ARROW_LEFT){
        if(cursor_pos > 0 && cursor_pos > ((cursor_pos / (SCREEN_COLS * 2)) * (SCREEN_COLS * 2)) + get_driver_name_size()*2){
            cursor_pos -= 2;
            if(!on_shift){
                index_of_shift = -1;
            }
            update_screen();
        }
    }

    else if( scan_code ==ARROW_RIGHT){
        if(cursor_pos < last_pos){
            cursor_pos += 2;
            if(!on_shift){
                index_of_shift = -1;
            }
            update_screen();
        }
    }

    else if( scan_code == ARROW_UP){
        
        display_history_index--;
        if(display_history_index < 0){
            display_history_index = 0;
            return;
        }
        get_from_history(display_history_index);
    }

    else if( scan_code == ARROW_DOWN){
        
        display_history_index++;
        if(display_history_index > size_of_history ){
            display_history_index = size_of_history;
            return;
        }
        get_from_history(display_history_index);
    }
    else if( scan_code == CONTROL_ON){
        on_control = 1;
        return;
    }
    else if( scan_code == CONTROL_OFF){
        on_control = 0;
        return;
    }

    else if(on_control == 1 && (scan_code == SCAN_CODE_C )){ // Ctrl + C
        control_c_handler();
        return;
    }

    else if(on_control == 1 && scan_code == SCAN_CODE_V){// Ctrl + V
        control_v_handler();
        update_cursor();
        update_screen();
        refresh_cmd_buffer();
        index_of_shift = -1;
        return;
    }

    else if(scan_code < 0x80 && index_of_shift != -1){
        int start = min(cursor_pos, index_of_shift)/2;
        int end = max(cursor_pos, index_of_shift)/2;

         // אם יש טקסט מסומן 
        if(start != end){
            delete_txt_on_shift();
            index_of_shift = -1;
            if(keyboard_map[scan_code] == '\b'){
                return;
            }
        }
        update_cursor();
        update_screen();
        refresh_cmd_buffer();
        handle_normal_press(scan_code);
    }
    else{
        handle_normal_press(scan_code);
    }
    refresh_cmd_buffer();
    update_cursor();
}

void control_c_handler(){
    if (index_of_shift == -1) return; // אין מה להעתיק

    int start = min(cursor_pos, index_of_shift)/2;
    int end = max(cursor_pos, index_of_shift)/2;
    int size = 0;

    for (int i = start; i < end ; i++) {
        copy_text_buffer[size++] = (char)(terminal_buffer[i] & 0xFF);
    }
    copy_text_buffer[size] = null;
}

void control_v_handler(){
    delete_txt_on_shift(); // מחיקת הטקסט המסומן אם יש
    for(int i = 0; copy_text_buffer[i] != null; i++){
        print_char(copy_text_buffer[i]);
    }
}

void delete_txt_on_shift(){
    if(index_of_shift == -1) return; // אין מה למחוק

    int times = max(cursor_pos, index_of_shift)/2 - min(cursor_pos, index_of_shift)/2;
    cursor_pos = max(cursor_pos, index_of_shift);
    for(int i = 0; i < times; i++){
        print_char('\b');
    }
}

void clean_row_of_terminal(){
    for(int i = get_safe_index(); i <= last_pos; i++){
        terminal_buffer[i] = ' ' | (cur_color << 8);
    }
    cursor_pos = get_safe_index()*2;
    last_pos = cursor_pos;
    update_screen();
}




void handle_normal_press(char scan_code){
    if (scan_code < 0x80) { //  (ללא העזיבה של המקש)רק לחיצות 
        char letter;
        if(on_shift == 1 || no_caps_lock == 0){
            letter = shift_keyboard_map[scan_code];
        }
        else{
            letter = keyboard_map[scan_code];
        }
        
        if (letter != 0) { // אם זה תו בעל תמיכה 

            print_char(letter);

            if(letter == '\n'){
                handle_commend();        // מעבד את הפקודה
                header_msg();            // מדפיס שורת פקודה חדשה
                wait();                  // מחכה קצת כדי למנוע בעיות של קלט מהיר מדי    
            }
        }
        update_cursor();
        update_screen();
    }
}
