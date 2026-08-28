#include "stdio.h"
#include "terminal_command.h"
#include "key_board.h"
#include "timer.h"

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
int get_safe_index();
void wait();  


// לתווים בעת לחיצה על המקלדת hex תרגום מ
// 0 --> מקש שאין לו טיפול
unsigned char keyboard_map[128] = {
    0,  0, '1', '2', '3', '4', '5', '6', '7', '8',
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
    0, 0, '!', '@', '#', '$', '%', '^', '&', '*', '(', ')', '_', '+', '\b', 
    '\t', 'Q', 'W', 'E', 'R', 'T', 'Y', 'U', 'I', 'O', 'P', '{', '}', '\n', 
    0, 'A', 'S', 'D', 'F', 'G', 'H', 'J', 'K', 'L', ':', '\"', '~', 
    0, '|', 'Z', 'X', 'C', 'V', 'B', 'N', 'M', '<', '>', '?', 0, 0, 0, ' '
};

KeyBoard key_board = {
    .index_of_shift = -1,
    .on_shift = 0,
    .on_control = 0,
    .no_caps_lock = 1,
    .keyboard_map = keyboard_map,
    .shift_keyboard_map = shift_keyboard_map,
    .copy_text_buffer = {null}  
};


void key_board_handler() {

    unsigned char scan_code = port_byte_in(0x60);

    if (scan_code == ARROW_UP && key_board.on_shift == 1)
    {
        if (print_info.current_view_row + print_info.buffer_row_offset > 0)
        {
            print_info.buffer_row_offset += 1;
            print_info.buffer_row_offset = min(print_info.buffer_row_offset, print_info.current_view_row);
        }
    }

    else if (scan_code == ARROW_DOWN && key_board.on_shift == 1)
    {
        if (print_info.buffer_row_offset > 0)
        {
            print_info.buffer_row_offset -= 1;
        }
    }
    else{

        if(scan_code < 0x80 && scan_code != ON_SHIFT_1 && scan_code != ON_SHIFT_2 && scan_code != OFF_SHIFT_1 && scan_code != OFF_SHIFT_2 && scan_code != CONTROL_ON && scan_code != CONTROL_OFF){
            print_info.buffer_row_offset = 0;
        }
    }

    //shift טיפול בעזיבה של המקש 
    if(scan_code == OFF_SHIFT_1 || scan_code == OFF_SHIFT_2){
        key_board.on_shift = 0;
    }
    //shift טיפול בלחיצה על מקש
    else if (scan_code == ON_SHIFT_1 || scan_code == ON_SHIFT_2) {
        key_board.on_shift = 1;
        key_board.index_of_shift = print_info.cursor_pos;
    }
    //caps lock טיפול בלחיצה על מקש
    else if(scan_code == CAP_LOCK_ON){
        key_board.no_caps_lock = !key_board.no_caps_lock;
    }

    else if( scan_code == ARROW_LEFT){
        if(print_info.cursor_pos > 0 && print_info.cursor_pos > get_safe_index()*2){
            print_info.cursor_pos -= 2;
            if(!key_board.on_shift){
                key_board.index_of_shift = -1;
            }
            update_screen();
        }
    }

    else if( scan_code == ARROW_RIGHT){
        if(print_info.cursor_pos < print_info.last_pos){
            print_info.cursor_pos += 2;
            if(!key_board.on_shift){
                key_board.index_of_shift = -1;
            }
            update_screen();
        }
    }


    else if( scan_code == ARROW_UP && key_board.on_shift == 0){

        if (terminal_command.display_history_index > 0)
        {
            terminal_command.display_history_index--;
            get_from_history(terminal_command.display_history_index);
        }
    }

    else if( scan_code == ARROW_DOWN && key_board.on_shift == 0){

        if (terminal_command.display_history_index < terminal_command.size_of_history - 1)
        {
            terminal_command.display_history_index++;
            get_from_history(terminal_command.display_history_index);
            update_cursor();
        }
        else if (terminal_command.display_history_index == terminal_command.size_of_history - 1)
        {
            terminal_command.display_history_index++;
            clean_command_buffer();
            clean_row_of_terminal();
            update_cursor();
        }
    }

    else if( scan_code == CONTROL_ON){
        key_board.on_control = 1;
    }
    
    else if( scan_code == CONTROL_OFF){
        key_board.on_control = 0;
    }

    else if(key_board.on_control == 1 && (scan_code == C_KEY_UP )){ // Ctrl + C
        control_c_handler();
    }

    else if(key_board.on_control == 1 && scan_code == V_key_UP){// Ctrl + V
        control_v_handler();
        update_screen();
        update_cursor();
        refresh_cmd_buffer();
        key_board.index_of_shift = -1;
    }
    else if(key_board.on_control == 1 && scan_code == A_KEY_UP){ // Ctrl + A
        key_board.index_of_shift = get_safe_index()*2;
        print_info.cursor_pos = print_info.last_pos;
    }

    else if(scan_code < 0x80 && key_board.index_of_shift != -1){
        int start = min(print_info.cursor_pos, key_board.index_of_shift)/2;
        int end = max(print_info.cursor_pos, key_board.index_of_shift)/2;

         // אם יש טקסט מסומן 
        if(start != end-1){
            delete_txt_on_shift();
            key_board.index_of_shift = -1;
            if(key_board.keyboard_map[scan_code] == '\b'){
                return;
            }
        }
        update_cursor();
        update_screen();
        refresh_cmd_buffer();
        handle_normal_press(scan_code);
    }
    else if(scan_code < 0x80){
        handle_normal_press(scan_code);
    }
    
    refresh_cmd_buffer();
    update_cursor();
    update_screen();

    port_byte_out(0x20, 0x20); // EOI --> משחרר את ההאזנה על המקלדת כדי להמשיך לקלוט מקשים
}

void control_c_handler()
{
    if (key_board.index_of_shift == -1)
        return; // אין מה להעתיק

    int start = min(print_info.cursor_pos, key_board.index_of_shift) / 2;
    int end = max(print_info.cursor_pos, key_board.index_of_shift) / 2;
    int size = 0;

    for (int i = start; i < end && size < COPY_TEXT_BUFFER_SIZE; i++)
    {
        key_board.copy_text_buffer[size++] = (char)(print_info.terminal_buffer[i] & 0xFF);
    }
    key_board.copy_text_buffer[size] = null;
}

void control_v_handler()
{
    delete_txt_on_shift();
    for (int i = 0; key_board.copy_text_buffer[i] != null; i++)
    {
        printf("%c", key_board.copy_text_buffer[i]);
    }
}

void delete_txt_on_shift()
{
    if (key_board.index_of_shift == -1)
        return; // אין מה למחוק

    int target_pos = min(print_info.cursor_pos, key_board.index_of_shift);

    print_info.cursor_pos = max(print_info.cursor_pos, key_board.index_of_shift);

    while (print_info.cursor_pos > target_pos && (print_info.cursor_pos / 2) > get_safe_index())
    {
        printf("\b");
    }
    key_board.index_of_shift = -1;
}


void clean_row_of_terminal(){
    for(int i = get_safe_index(); i <= print_info.last_pos/2; i++){
        print_info.terminal_buffer[i] = ' ' | (WHITE_ON_BLACK << 8);
    }
    print_info.cursor_pos = get_safe_index()*2;
    print_info.last_pos = print_info.cursor_pos;
    update_screen();
}
    

void handle_normal_press(char scan_code){

    if (scan_code < 0x80) { //  (ללא העזיבה של המקש)רק לחיצות 
        char letter;
        if(key_board.on_shift == 1 ^ key_board.no_caps_lock == 0){
            letter = key_board.shift_keyboard_map[scan_code];
        }
        else{
            letter = key_board.keyboard_map[scan_code];
        }
        
        if (letter != 0) { // אם זה תו בעל תמיכה 

            printf("%c", letter);

            if(letter == '\n'){
                handle_command();        // מעבד את הפקודה
                header_msg();            // מדפיס שורת פקודה חדשה
                wait();                
            }
        }
        update_cursor();
        update_screen();
    }
}


void wait(){
    for(int i = 0; i< 1000000; i++){

    }
}
