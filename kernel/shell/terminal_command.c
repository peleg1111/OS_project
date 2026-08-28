#include "stdio.h"
#include "terminal_command.h"
#include "key_board.h"

void update_screen();
void set_color(int color);
void lower(char* c);
void printf(char* msg, ...);

Terminal_command terminal_command = {
    .cmd_buffer = {0},
    .size_of_history = 0,
    .commands_history = {{0}},
    .display_history_index = 0,
    .cmd_index = 0,
    .handle_command = null,
    .clean_command_buffer = null,
    .change_color = null,
    .clean_command_history = null,
    .insert_to_history = null,
    .display_history = null
};



void terminal_init(){

    terminal_command.handle_command = handle_command;
    terminal_command.clean_command_buffer = clean_command_buffer;
    terminal_command.change_color = change_color;
    terminal_command.clean_command_history = clean_command_history;
    terminal_command.insert_to_history = insert_to_history;
    terminal_command.display_history = display_history;

}

void clean_command_buffer() {
    for(int i = 0; i < COMMAND_BUFFER_SIZE; i++) {
        terminal_command.cmd_buffer[i] = null;
    }
    terminal_command.cmd_index = 0;
}


void handle_command() {
    insert_to_history(terminal_command.cmd_buffer);
    terminal_command.display_history_index = terminal_command.size_of_history;

    // פירוק הפקודה
    char* parts[10];
    char* c = strip(terminal_command.cmd_buffer);
    int size = split(c , ' ' , parts);

    if (size == 0) return; // פקודה ריקה

    // ההדפסה של הפקודה
    printf("cmd got -->> ");
    for(int i = 0; i < size ; i++){
        lower(parts[i]);
        printf("$%s$",parts[i]);
    }

    // הניתוח של הפקודה
    if(strcmp(parts[0], "clear") == 1){
        clear_screen();
    }

    else if(strcmp(parts[0] , "color")==1 ){
        change_color(parts, size);
    }


    else if (strcmp(parts[0] , "history") == 1){
        display_history();
    }

    else if(strcmp(parts[0] , "help" ) == 1){

        printf(
            "\n\n========================= TERMINAL HELP ========================="
            "\n[ USAGE ]"
            "\n  Type a command name followed by its arguments and press ENTER."
            "\n"
            "\n[ AVAILABLE COMMANDS ]"
            "\n  clear                 - Clears the terminal screen."
            "\n  color <txt color_name> <bg color_name (optional)>    - Changes text color."
            "\n                          Available: green, yellow, white, blue"
            "\n  history               - Shows the list of previously used commands."
            "\n  help                  - Displays this help menu."
            "\n"
            "\n[ KEYBOARD SHORTCUTS ]"
            "\n  UP / DOWN             - Navigate command history."
            "\n  LEFT / RIGHT          - Move cursor back and forth."
            "\n  Shift + UP / DOWN     - Scroll up / down through buffer."
            "\n  Shift + LEFT / RIGHT  - Select / highlight text."
            "\n  Ctrl + A              - Select all text on current row."
            "\n  Ctrl + C              - Copy selected text."
            "\n  Ctrl + V              - Paste copied text."
            "\n================================================================="
            "\n");
    }

    else{
        int old_color = print_info.cur_color;
        set_color(RED_ON_BLACK);
        printf("\nUnknown command: %s", parts[0]);
        set_color(old_color);
    }
    // איפוס הפקודה לאחר הלוגיקה שלה
    clean_command_buffer();
}


void print_to_command_buffer(char c) {

    if (terminal_command.cmd_index >= COMMAND_BUFFER_SIZE - 1){
        clean_command_buffer();
        clean_row_of_terminal();
        int old_color = print_info.cur_color;
        set_color(RED_ON_BLACK);
        printf("\nerror --> too long command\n");
        set_color(old_color);
        header_msg();
        print_info.buffer_row_offset = 0;
        update_cursor();
        update_screen();
        return;
    }

    if (c == '\n') {
        terminal_command.cmd_buffer[terminal_command.cmd_index] = null;
        terminal_command.display_history_index = terminal_command.size_of_history;
        return; 
    }

    else if(c == '\t'){
        print_to_command_buffer(' ');
        print_to_command_buffer(' ');
        print_to_command_buffer(' ');
        print_to_command_buffer(' ');
    }
    else if( c == '\b'){
        terminal_command.cmd_buffer[terminal_command.cmd_index] = null;
        terminal_command.cmd_index--;
        if(terminal_command.cmd_index < 0){
            terminal_command.cmd_index = 0;
        }
    }
    else{
        if (terminal_command.cmd_index < COMMAND_BUFFER_SIZE - 1) {
            terminal_command.cmd_buffer[terminal_command.cmd_index] = c;
            terminal_command.cmd_index++;
        }
    }
}

int get_color_by_string(char* str){

    if (str == null)
        return -1;

    if (strcmp(str, "black") == 1)
        return BLACK_ON_BLACK;
    if (strcmp(str, "blue") == 1)
        return BLUE_ON_BLACK;
    if (strcmp(str, "green") == 1)
        return GREEN_ON_BLACK;
    if (strcmp(str, "cyan") == 1)
        return CYAN_ON_BLACK;
    if (strcmp(str, "red") == 1)
        return RED_ON_BLACK;
    if (strcmp(str, "magenta") == 1)
        return MAGENTA_ON_BLACK;
    if (strcmp(str, "brown") == 1)
        return BROWN_ON_BLACK;
    if (strcmp(str, "light_gray") == 1)
        return LIGHT_GRAY_ON_BLACK;
    if (strcmp(str, "dark_gray") == 1)
        return DARK_GRAY_ON_BLACK;
    if (strcmp(str, "light_blue") == 1)
        return LIGHT_BLUE_ON_BLACK;
    if (strcmp(str, "light_green") == 1)
        return LIGHT_GREEN_ON_BLACK;
    if (strcmp(str, "light_cyan") == 1)
        return LIGHT_CYAN_ON_BLACK;
    if (strcmp(str, "light_red") == 1)
        return ORANGE_ON_BLACK;
    if (strcmp(str, "light_magenta") == 1)
        return LIGHT_MAGENTA_ON_BLACK;
    if (strcmp(str, "yellow") == 1)
        return YELLOW_ON_BLACK;
    if (strcmp(str, "white") == 1)
        return WHITE_ON_BLACK;

    return -1; // צבע לא נמצא
}

void change_color(char* parts[], int size){

    if(size < 3){
        printf("\nUsage: color <txt color_name> <bg color_name>");
        return;
    }
    int color = get_color_by_string(parts[1]);
    if(color == -1){

        int old_color = print_info.cur_color;
        set_color(RED_ON_BLACK);
        printf("\nUnknown color: %s", parts[1]);
        set_color(old_color);
        return;
    }

    int bg = get_color_by_string(parts[2]);
    if(bg == -1){
        int old_color = print_info.cur_color;
        set_color(RED_ON_BLACK);
        printf("\nUnknown bg color: %s", parts[2]);
        set_color(old_color);
        return;
    }
    color += (bg << 4);
    set_color(color);
    
}

void clean_command_history(){

    for(int i = 0; i < terminal_command.size_of_history; i++){
        terminal_command.commands_history[i][0] = null;
    }
    terminal_command.size_of_history = 0;
}

void insert_to_history(char* cmd_buffer){
    if(cmd_buffer[0] == null) return; // פקודה ריקה לא נכנסת להיסטוריה
    if (terminal_command.size_of_history > 0 && strcmp(cmd_buffer, terminal_command.commands_history[terminal_command.size_of_history - 1]) == 1)
        return; // פקודה זהה לפקודה האחרונה לא נכנסת להיסטוריה

    if(terminal_command.size_of_history < SCREEN_ROWS * MAX_ROWS && terminal_command.size_of_history >= 0 ){
        
    int i = 0;
    while (cmd_buffer[i] != null && i < SCREEN_ROWS * MAX_ROWS -1) {
        terminal_command.commands_history[terminal_command.size_of_history][i] = cmd_buffer[i];
        i++;
    }

    terminal_command.commands_history[terminal_command.size_of_history][i] = null;
        terminal_command.size_of_history++;
        terminal_command.display_history_index = terminal_command.size_of_history;
    }
}

void display_history(){

    printf("\nCommand History:\n");
    for(int i = 0; i < terminal_command.size_of_history; i++){
        if(terminal_command.commands_history[i][0] != null){
            printf("\n--- %s", terminal_command.commands_history[i]);
        }
    }
}

void get_from_history(int index){
    key_board.index_of_shift = -1;

    if (index != -1 && index > -1 && index < terminal_command.size_of_history)
    {
        clean_row_of_terminal();
        printf("%s", terminal_command.commands_history[index]);
        refresh_cmd_buffer();
    }
    update_cursor();
}

// יוצר אותו מחדש לאחר שינוי של הפקודה חיצונית 
void refresh_cmd_buffer(){
    clean_command_buffer();
    for(int i = get_safe_index(); i < print_info.last_pos/2; i++){
        char c = (char)(print_info.terminal_buffer[i] & 0xFF);
         if (c != null) {
            print_to_command_buffer(c);
         }
    }
}
